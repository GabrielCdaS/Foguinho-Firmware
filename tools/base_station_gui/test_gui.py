import sys
import os
import struct

# Adicionar pasta local ao path para garantir importações corretas
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from PySide6.QtWidgets import QApplication
from telemetry_parser import TelemetryParser, pack_command
from mock_serial import MockSerial
from main import BaseStationGUI

def run_tests():
    print("=== Iniciando Testes Unitários e Integração ===")

    # 1. Testar Parser e Desempacotamento de Telemetria
    print("1. Testando máquina de estados do TelemetryParser...")
    parser = TelemetryParser()
    mock = MockSerial()
    mock.open()

    # Espera gerar alguns bytes de telemetria simulada
    import time
    time.sleep(0.15)
    mock._update_simulation()
    in_waiting = mock.in_waiting
    print(f"   Bytes no buffer simulado: {in_waiting}")
    assert in_waiting >= 34, "Erro: simulador não gerou bytes suficientes."

    data = mock.read(in_waiting)
    
    # Alimenta o parser byte a byte
    parsed_telemetry = None
    crc_errors = 0
    
    for b in data:
        res = parser.feed_byte(b)
        if res:
            if res.get("type") == "TELEMETRY":
                parsed_telemetry = res
                break
            elif res.get("type") == "CRC_ERROR":
                crc_errors += 1

    assert crc_errors == 0, f"Erro: Detectado {crc_errors} erro(s) de CRC durante telemetria normal."
    assert parsed_telemetry is not None, "Erro: Parser falhou em reconstruir pacote de telemetria válido."
    
    print("   Pacote de telemetria decodificado:")
    for k, v in parsed_telemetry.items():
        if k != "type":
            print(f"     - {k}: {v}")
            
    assert parsed_telemetry["header"] == 0xAA, "Erro: Header incorreto."
    assert parsed_telemetry["flight_state"] == "IDLE", "Erro: Estado inicial deveria ser IDLE."
    assert abs(parsed_telemetry["acceleration_g"] - 1.0) < 0.2, "Erro: Aceleração fora da gravidade normal."

    # 2. Testar Empacotamento de Comandos e Resposta
    print("2. Testando envio de comandos e recepção de respostas...")
    # Prepara comando de ARM (0x01)
    cmd_packet = pack_command(0x01)
    assert len(cmd_packet) == 5, f"Erro: Pacote de comando deveria ter 5 bytes, tem {len(cmd_packet)}"
    assert cmd_packet[0] == 0xAA, "Erro: Header de comando incorreto."
    assert cmd_packet[1] == 0x01, "Erro: ID de comando incorreto."

    # Envia comando para o Mock
    mock.write(cmd_packet)
    mock._update_simulation()

    # Verifica se gerou resposta
    resp_waiting = mock.in_waiting
    assert resp_waiting >= 5, f"Erro: Mock não respondeu ao comando. Bytes disponíveis: {resp_waiting}"
    
    resp_data = mock.read(resp_waiting)
    
    # Configura parser para esperar resposta de ARM (0x01)
    parser.expecting_cmd_id = 0x01
    command_response = None
    
    for b in resp_data:
        res = parser.feed_byte(b)
        if res and res.get("type") == "COMMAND_RESPONSE":
            command_response = res
            break

    assert command_response is not None, "Erro: Parser falhou em extrair resposta de comando."
    print(f"   Resposta de comando decodificada: {command_response}")
    assert command_response["command_id"] == 0x01, "Erro: ID de resposta incorreto."
    assert command_response["result"] == "OK", f"Erro: Resultado deveria ser OK, obtido {command_response['result']}"

    # Testar se o estado do Mock mudou para ARMED (2)
    time.sleep(0.15)
    mock._update_simulation()
    new_data = mock.read(mock.in_waiting)
    new_telemetry = None
    for b in new_data:
        res = parser.feed_byte(b)
        if res and res.get("type") == "TELEMETRY":
            new_telemetry = res
            break
            
    assert new_telemetry is not None
    print(f"   Novo estado do foguete após ARM: {new_telemetry['flight_state']}")
    assert new_telemetry["flight_state"] == "ARMED", "Erro: Foguete não armou corretamente."

    # 3. Testar Instanciação da GUI (Modo Headless/Sem Loops de Eventos Bloqueantes)
    print("3. Testando instanciação do painel Qt (BaseStationGUI)...")
    app = QApplication.instance() or QApplication([])
    gui = BaseStationGUI()
    assert gui is not None, "Erro ao instanciar BaseStationGUI."
    assert gui.windowTitle() == "ESTAÇÃO DE BASE - FOGUINHO 🚀", "Erro: Título da janela incorreto."
    assert gui.data_tabs.count() == 2, "Erro: A aba de visualização GPS não foi criada."
    assert not gui.btn_open_map.isEnabled(), "Erro: Botão do mapa deveria iniciar desabilitado."

    gui.on_packet_received(parsed_telemetry)
    assert gui.lbl_gps_details.text().startswith("FIX ATIVO"), "Erro: Dados GPS não foram exibidos."
    assert len(gui.gps_lat_history) == 1, "Erro: Posição GPS válida não entrou no trajeto."
    assert gui.btn_open_map.isEnabled(), "Erro: Botão do mapa não foi habilitado após um fix válido."
    gui.close()
    
    print("\n[SUCCESS] Todos os testes passaram com sucesso! O visualizador esta pronto para uso.")
    return 0

if __name__ == "__main__":
    sys.exit(run_tests())
