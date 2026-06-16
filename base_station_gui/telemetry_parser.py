import struct
from typing import Dict, Any, Optional, Tuple

# Constantes do protocolo
PROTOCOL_HEADER_BYTE = 0xAA
PROTOCOL_PACKET_SIZE = 34
COMMAND_PACKET_SIZE = 5

# Mapeamento dos estados de voo da FSM
FLIGHT_STATES = {
    0: "BOOT",
    1: "IDLE",
    2: "ARMED",
    3: "ASCENT",
    4: "APOGEE",
    5: "DESCENT",
    6: "LANDED"
}

# Mapeamento dos comandos
COMMAND_IDS = {
    0x01: "ARM",
    0x02: "DISARM",
    0x03: "PING",
    0x04: "START_LOG",
    0x05: "STOP_LOG"
}

# Mapeamento dos resultados de comandos
COMMAND_RESULTS = {
    0: "OK",
    1: "INVALID_STATE",
    2: "SAFETY_LOCK",
    3: "STORAGE_ERROR",
    4: "HARDWARE_ERROR"
}

def crc16_ccitt(data: bytes) -> int:
    """
    Calcula o CRC-16/CCITT (polinômio 0x1021, valor inicial 0xFFFF).
    Mesmo algoritmo implementado no firmware (crc16.c).
    """
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

class TelemetryParser:
    """
    Parser de telemetria orientado a bytes (Máquina de Estados).
    Mimetiza a lógica de telemetry_parser.h do firmware da estação base.
    """
    # Estados da máquina do parser
    STATE_WAIT_HEADER = 0
    STATE_RECEIVING = 1

    def __init__(self):
        self.state = self.STATE_WAIT_HEADER
        self.buffer = bytearray()
        self.valid_packets_count = 0
        self.crc_errors_count = 0
        self.expecting_cmd_id = None

    def reset(self):
        self.state = self.STATE_WAIT_HEADER
        self.buffer.clear()
        self.expecting_cmd_id = None

    def feed_byte(self, byte: int) -> Optional[Dict[str, Any]]:
        """
        Alimenta o parser com um único byte.
        Retorna o dicionário de dados da telemetria/resposta se um pacote válido for completado,
        ou None caso contrário.
        """
        if self.state == self.STATE_WAIT_HEADER:
            if byte == PROTOCOL_HEADER_BYTE:
                self.buffer.clear()
                self.buffer.append(byte)
                self.state = self.STATE_RECEIVING
        elif self.state == self.STATE_RECEIVING:
            self.buffer.append(byte)
            
            # Verificar se é uma resposta de comando (5 bytes)
            if self.expecting_cmd_id is not None and len(self.buffer) == 5:
                if self.buffer[1] == self.expecting_cmd_id:
                    payload = bytes(self.buffer[:3])
                    received_crc = struct.unpack("<H", self.buffer[3:5])[0]
                    calculated_crc = crc16_ccitt(payload)
                    if calculated_crc == received_crc:
                        cmd_id = self.buffer[1]
                        result_id = self.buffer[2]
                        self.state = self.STATE_WAIT_HEADER
                        self.expecting_cmd_id = None
                        return {
                            "type": "COMMAND_RESPONSE",
                            "command_id": cmd_id,
                            "command": COMMAND_IDS.get(cmd_id, f"UNKNOWN ({cmd_id})"),
                            "result_id": result_id,
                            "result": COMMAND_RESULTS.get(result_id, f"UNKNOWN ({result_id})")
                        }
            
            # Verificar se é um pacote de telemetria completo (34 bytes)
            if len(self.buffer) == PROTOCOL_PACKET_SIZE:
                packet_data = bytes(self.buffer)
                # Voltar ao estado inicial para o próximo byte
                self.state = self.STATE_WAIT_HEADER
                
                # Validar CRC-16
                payload = packet_data[:32]
                received_crc = struct.unpack("<H", packet_data[32:34])[0]
                calculated_crc = crc16_ccitt(payload)
                
                if calculated_crc == received_crc:
                    self.valid_packets_count += 1
                    data = self.unpack_telemetry(packet_data)
                    data["type"] = "TELEMETRY"
                    return data
                else:
                    self.crc_errors_count += 1
                    # Retorna um dicionário indicando erro de CRC para logs
                    return {"type": "CRC_ERROR", "expected": calculated_crc, "received": received_crc}
        return None

    @staticmethod
    def unpack_telemetry(packet: bytes) -> Dict[str, Any]:
        """
        Desempacota a estrutura binária de 34 bytes da telemetria.
        """
        # Formato little-endian (<):
        # B: header (1B)
        # B: packet_id (1B)
        # I: timestamp_ms (4B)
        # f: altitude_m (4B)
        # f: vert_velocity_ms (4B)
        # f: acceleration_g (4B)
        # H: battery_mv (2B)
        # B: flight_state (1B)
        # i: gps_latitude (4B)
        # i: gps_longitude (4B)
        # h: gps_altitude_m (2B)
        # B: gps_info (1B)
        # H: crc16 (2B)
        fmt = "<BBIfffHBiihBH"
        unpacked = struct.unpack(fmt, packet)
        
        # Extrair campos de GPS de gps_info
        gps_info_byte = unpacked[11]
        gps_fix = bool(gps_info_byte & 0x80)
        gps_sats = gps_info_byte & 0x7F

        return {
            "header": unpacked[0],
            "packet_id": unpacked[1],
            "timestamp_ms": unpacked[2],
            "altitude_m": unpacked[3],
            "vert_velocity_ms": unpacked[4],
            "acceleration_g": unpacked[5],
            "battery_mv": unpacked[6],
            "battery_v": unpacked[6] / 1000.0,
            "flight_state_id": unpacked[7],
            "flight_state": FLIGHT_STATES.get(unpacked[7], f"UNKNOWN ({unpacked[7]})"),
            "gps_latitude": unpacked[8] / 1e7,
            "gps_longitude": unpacked[9] / 1e7,
            "gps_altitude_m": unpacked[10],
            "gps_fix": gps_fix,
            "gps_satellites": gps_sats,
            "crc16": unpacked[12]
        }

def pack_command(command_id: int, payload_byte: int = 0) -> bytes:
    """
    Empacota um comando de 5 bytes para enviar ao foguete:
    [Header (0xAA), Command ID (1B), Payload (1B), CRC-16 (2B)]
    """
    raw_packet = struct.pack("<BBB", PROTOCOL_HEADER_BYTE, command_id, payload_byte)
    crc = crc16_ccitt(raw_packet)
    return raw_packet + struct.pack("<H", crc)
