import time
import struct
import math
from typing import Optional
from telemetry_parser import (
    PROTOCOL_HEADER_BYTE,
    crc16_ccitt,
    FLIGHT_STATES,
    COMMAND_IDS,
    COMMAND_RESULTS
)

class MockSerial:
    """
    Simulador de porta serial em Python.
    Simula uma telemetria realística de voo de foguete e responde a comandos.
    """
    def __init__(self, port: str = "MOCK0", baudrate: int = 115200, timeout: Optional[float] = None):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self._is_open = True
        
        # Estado do simulador
        self.start_time = time.time()
        self.last_packet_time = time.time()
        self.packet_interval = 0.1  # 10 Hz
        
        # Buffer de recebimento (dados que o PC lê do rádio)
        self.rx_buffer = bytearray()
        
        # Parâmetros de voo simulados
        self.packet_id = 0
        self.flight_state = 1  # FSM_IDLE (1 = IDLE, 2 = ARMED, 3 = ASCENT, 4 = APOGEE, 5 = DESCENT, 6 = LANDED)
        self.altitude = 0.0
        self.velocity = 0.0
        self.acceleration = 1.0  # 1g em repouso
        self.battery_mv = 8400   # 8.4V nominal
        
        # GPS inicial (próximo de um local fictício de lançamento)
        self.gps_lat_init = -23.550520 * 1e7  # São Paulo
        self.gps_lon_init = -46.633309 * 1e7
        self.gps_lat = int(self.gps_lat_init)
        self.gps_lon = int(self.gps_lon_init)
        self.gps_alt = 760  # metros
        self.gps_sats = 8
        self.gps_fix = 1
        
        # Controle de voo automático (simulado)
        self.launched = False
        self.launch_time = 0.0
        self.apogee_time = 0.0
        self.landed_time = 0.0
        self.logging_active = False

    @property
    def is_open(self) -> bool:
        return self._is_open

    def open(self):
        self._is_open = True
        self.start_time = time.time()
        self.last_packet_time = time.time()

    def close(self):
        self._is_open = False

    @property
    def in_waiting(self) -> int:
        self._update_simulation()
        return len(self.rx_buffer)

    def read(self, size: int = 1) -> bytes:
        if not self._is_open:
            raise Exception("Port is closed")
            
        self._update_simulation()
        
        if not self.rx_buffer:
            # Se for leitura bloqueante, aguarda um pouco
            if self.timeout and self.timeout > 0:
                time.sleep(min(self.timeout, 0.01))
                self._update_simulation()
            else:
                return b""
                
        data_to_return = self.rx_buffer[:size]
        del self.rx_buffer[:size]
        return bytes(data_to_return)

    def write(self, data: bytes) -> int:
        """
        Recebe comando enviado pelo PC.
        Valida o CRC do comando e coloca a resposta correspondente no buffer RX.
        """
        if not self._is_open:
            raise Exception("Port is closed")
            
        # Verifica se é um comando válido (5 bytes)
        if len(data) == 5 and data[0] == PROTOCOL_HEADER_BYTE:
            cmd_id = data[1]
            payload = data[2]
            received_crc = struct.unpack("<H", data[3:5])[0]
            calculated_crc = crc16_ccitt(data[:3])
            
            if calculated_crc == received_crc:
                # CRC OK, processar comando
                result = 0  # COMMAND_RESULT_OK
                
                if cmd_id == 0x01:  # ARM
                    if self.flight_state == 1:  # IDLE
                        self.flight_state = 2  # ARMED
                        # Iniciar contagem para lançamento automático
                        self.launch_time = time.time() + 3.0  # lança em 3 segundos
                        result = 0
                    else:
                        result = 1  # COMMAND_RESULT_INVALID_STATE
                elif cmd_id == 0x02:  # DISARM
                    if self.flight_state in [1, 2]:  # IDLE ou ARMED
                        self.flight_state = 1  # Volta para IDLE
                        self.launched = False
                        result = 0
                    else:
                        result = 1  # COMMAND_RESULT_INVALID_STATE
                elif cmd_id == 0x03:  # PING
                    result = 0  # OK
                elif cmd_id == 0x04:  # START_LOG
                    self.logging_active = True
                    result = 0
                elif cmd_id == 0x05:  # STOP_LOG
                    self.logging_active = False
                    result = 0
                else:
                    result = 1  # Invalid command ID
                
                # Gerar resposta de 5 bytes
                # [Header, Command ID, Result, CRC16]
                resp_payload = struct.pack("<BBB", PROTOCOL_HEADER_BYTE, cmd_id, result)
                resp_crc = crc16_ccitt(resp_payload)
                resp_packet = resp_payload + struct.pack("<H", resp_crc)
                
                # Coloca a resposta no buffer rx
                self.rx_buffer.extend(resp_packet)
                
        return len(data)

    def flush(self):
        pass

    def _update_simulation(self):
        """
        Gera a telemetria do foguete baseada no tempo decorrido.
        """
        now = time.time()
        elapsed = now - self.start_time
        
        # Gera pacotes a 10 Hz
        time_since_last = now - self.last_packet_time
        if time_since_last >= self.packet_interval:
            packets_to_gen = int(time_since_last / self.packet_interval)
            for _ in range(packets_to_gen):
                self._generate_telemetry_packet(now)
                self.packet_id = (self.packet_id + 1) % 256
            self.last_packet_time = now

    def _generate_telemetry_packet(self, now: float):
        # Atualiza a física do foguete com base no estado atual
        # 1 = IDLE, 2 = ARMED, 3 = ASCENT, 4 = APOGEE, 5 = DESCENT, 6 = LANDED
        
        # Descarrega a bateria levemente ao longo do tempo
        self.battery_mv = max(6800, 8400 - int((now - self.start_time) * 5))
        
        # Se estiver em IDLE por mais de 5 segundos no simulador, auto-arma para fins de demonstração
        if self.flight_state == 1 and (now - self.start_time) > 5.0 and not self.launched:
            self.flight_state = 2  # ARMED
            self.launch_time = now + 2.0  # Decola em 2 segundos
            
        # Se armado e passou o tempo de lançamento, decola!
        if self.flight_state == 2 and self.launch_time > 0 and now >= self.launch_time:
            self.flight_state = 3  # ASCENT
            self.launched = True
            self.launch_time = now
            
        if self.flight_state == 3:  # Subida (ASCENT)
            t = now - self.launch_time
            if t < 4.0:
                # Queima do motor (aceleração forte)
                self.acceleration = 1.0 + (t * 2.5) + 4.0  # sobe até ~14g
                self.velocity += (self.acceleration - 1.0) * 9.8 * self.packet_interval
                self.altitude += self.velocity * self.packet_interval
            elif t < 6.0:
                # Fim da queima, subida inercial (gravidade puxa de volta)
                self.acceleration = -1.2  # desacelera pelo arrasto e gravidade
                self.velocity += (self.acceleration) * 9.8 * self.packet_interval
                self.altitude += self.velocity * self.packet_interval
            else:
                # Próximo do apogeu
                self.flight_state = 4  # APOGEE
                self.apogee_time = now
                
        elif self.flight_state == 4:  # Apogeu (APOGEE)
            # Fica no apogeu por 1 segundo antes de ejetar o paraquedas
            self.acceleration = -1.0
            self.velocity = 0.0
            if now - self.apogee_time > 1.0:
                self.flight_state = 5  # DESCENT
                
        elif self.flight_state == 5:  # Descida (DESCENT)
            # Paraquedas aberto: velocidade de descida terminal constante
            target_descent_speed = -6.5  # m/s
            self.velocity = self.velocity * 0.8 + target_descent_speed * 0.2
            self.acceleration = 1.0  # flutua sob gravidade compensada pelo paraquedas
            self.altitude += self.velocity * self.packet_interval
            
            # Drift de vento nas coordenadas GPS
            self.gps_lat += int(math.sin(now * 0.1) * 3)
            self.gps_lon += int(math.cos(now * 0.1) * 5)
            
            if self.altitude <= 0.0:
                self.altitude = 0.0
                self.velocity = 0.0
                self.acceleration = 1.0
                self.flight_state = 6  # LANDED
                self.landed_time = now
                
        elif self.flight_state == 6:  # Pousado (LANDED)
            self.altitude = 0.0
            self.velocity = 0.0
            self.acceleration = 1.0
            
            # Piscar coordenadas ligeiramente por ruído do GPS
            self.gps_lat += int(math.sin(now * 5.0) * 1)
            self.gps_lon += int(math.cos(now * 5.0) * 1)
            
        else:  # IDLE ou ARMED aguardando lançamento
            self.altitude = 0.0
            self.velocity = 0.0
            self.acceleration = 1.0
            # Pequeno ruído no barômetro (altitude)
            self.altitude = float(math.sin(now * 2.0) * 0.15)
            
        # Converter dados de física para o pacote de telemetria
        timestamp_ms = int((now - self.start_time) * 1000)
        
        # Coordenadas GPS formatadas
        self.gps_alt = int(self.altitude + 760)
        
        # Montar estrutura binária de 34 bytes
        # offset 0: header (uint8)
        # offset 1: packet_id (uint8)
        # offset 2: timestamp_ms (uint32)
        # offset 6: altitude_m (float)
        # offset 10: vert_velocity_ms (float)
        # offset 14: acceleration_g (float)
        # offset 18: battery_mv (uint16)
        # offset 20: flight_state (uint8)
        # offset 21: gps_latitude (int32)
        # offset 25: gps_longitude (int32)
        # offset 29: gps_altitude_m (int16)
        # offset 31: gps_info (uint8) -> bit 7: fix; bits 0-6: satelites
        # offset 32: crc16 (uint16)
        
        gps_info = (self.gps_fix << 7) | (self.gps_sats & 0x7F)
        
        payload = struct.pack(
            "<BBIfffHBiihB",
            PROTOCOL_HEADER_BYTE,
            self.packet_id,
            timestamp_ms,
            self.altitude,
            self.velocity,
            self.acceleration,
            self.battery_mv,
            self.flight_state,
            self.gps_lat,
            self.gps_lon,
            self.gps_alt,
            gps_info
        )
        
        # Calcular CRC-16
        crc = crc16_ccitt(payload)
        packet = payload + struct.pack("<H", crc)
        
        # Escrever no buffer rx
        self.rx_buffer.extend(packet)
