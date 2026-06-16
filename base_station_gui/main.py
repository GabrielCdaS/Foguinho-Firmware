import sys
import os
import time
import csv
from datetime import datetime
from typing import Optional
import serial
import serial.tools.list_ports
import pyqtgraph as pg

from PySide6.QtWidgets import (
    QApplication,
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QGridLayout,
    QGroupBox,
    QLabel,
    QPushButton,
    QComboBox,
    QCheckBox,
    QTextEdit,
    QFrame,
    QProgressBar,
    QSizePolicy,
    QTabWidget
)
from PySide6.QtCore import QThread, Signal, Slot, QTimer, Qt, QUrl
from PySide6.QtGui import QFont, QIcon, QColor, QDesktopServices

from telemetry_parser import TelemetryParser, pack_command, FLIGHT_STATES, COMMAND_IDS
from mock_serial import MockSerial
from styles import QSS_STYLE

class SerialWorker(QThread):
    packet_received = Signal(dict)
    log_message = Signal(str)
    status_changed = Signal(bool)

    def __init__(self, port, baudrate, is_mock=False):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.is_mock = is_mock
        self.running = True
        self.parser = TelemetryParser()
        self.ser = None
        self._cmd_queue = []

    def queue_command(self, cmd_id, payload=0):
        self._cmd_queue.append((cmd_id, payload))

    def run(self):
        try:
            if self.is_mock:
                self.ser = MockSerial(self.port, self.baudrate, timeout=0.1)
                self.log_message.emit(f"🚀 [MOCK] Conectado ao Simulador de Telemetria ({self.port})")
            else:
                self.ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
                self.log_message.emit(f"🔌 [SERIAL] Conectado à porta física {self.port} ({self.baudrate} bps)")
            
            self.status_changed.emit(True)
            self.parser.reset()
            
            while self.running:
                # 1. Envia comandos da fila
                while self._cmd_queue:
                    cmd_id, payload = self._cmd_queue.pop(0)
                    packet = pack_command(cmd_id, payload)
                    self.ser.write(packet)
                    self.parser.expecting_cmd_id = cmd_id
                    cmd_name = COMMAND_IDS.get(cmd_id, f"0x{cmd_id:02X}")
                    self.log_message.emit(f"➡️ [TX COMANDO] Enviado {cmd_name} | Pacote: {packet.hex().upper()}")

                # 2. Lê dados do serial
                try:
                    if self.is_mock or (self.ser.in_waiting > 0):
                        data = self.ser.read(64) if not self.is_mock else self.ser.read(self.ser.in_waiting or 1)
                        for b in data:
                            res = self.parser.feed_byte(b)
                            if res:
                                self.packet_received.emit(res)
                    else:
                        time.sleep(0.01)  # Descanso leve para a CPU
                except Exception as e:
                    self.log_message.emit(f"❌ [ERRO LEITURA] Porta desconectada ou falha física: {str(e)}")
                    break
                    
        except Exception as e:
            self.log_message.emit(f"❌ [ERRO CONEXÃO] Falha ao abrir a porta {self.port}: {str(e)}")
        finally:
            if self.ser:
                try:
                    self.ser.close()
                except:
                    pass
            self.status_changed.emit(False)
            self.log_message.emit("🔌 Porta serial fechada.")

class BaseStationGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ESTAÇÃO DE BASE - FOGUINHO 🚀")
        self.resize(1200, 800)
        self.setStyleSheet(QSS_STYLE)

        # Thread serial
        self.serial_worker = None
        self.is_connected = False

        # Arquivo de Log CSV
        self.csv_file = None
        self.csv_writer = None
        self.csv_filepath = None

        # Dados para Gráficos
        self.time_history = []
        self.alt_history = []
        self.vel_history = []
        self.acc_history = []
        self.gps_lat_history = []
        self.gps_lon_history = []
        self.last_valid_gps = None
        self.max_history_len = 300

        # Estatísticas do link
        self.packet_count = 0
        self.crc_errors = 0
        self.packet_rate_counter = 0
        self.packet_rate = 0.0

        # Inicializa Interface
        self.setup_ui()

        # Timer de 1 Hz para taxa de pacotes e atualização de timers
        self.sec_timer = QTimer()
        self.sec_timer.timeout.connect(self.update_second_stats)
        self.sec_timer.start(1000)

        # Timer para piscar alertas de FSM se necessário
        self.blink_state = False
        self.blink_timer = QTimer()
        self.blink_timer.timeout.connect(self.toggle_blink)
        self.blink_timer.start(500)

        self.log("💡 Interface da estação de base inicializada com sucesso.")
        self.refresh_ports()

    def setup_ui(self):
        # Widget Central
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(12, 12, 12, 12)
        main_layout.setSpacing(10)

        # -------------------------------------------------------------
        # 1. BARRA SUPERIOR (CONEXÃO)
        # -------------------------------------------------------------
        header_frame = QFrame()
        header_frame.setObjectName("cardFrame")
        header_layout = QHBoxLayout(header_frame)
        header_layout.setContentsMargins(10, 8, 10, 8)

        # Título
        title_lbl = QLabel("ESTAÇÃO BASE")
        title_lbl.setStyleSheet("font-weight: bold; font-size: 16px; color: #f43f5e;")
        header_layout.addWidget(title_lbl)

        header_layout.addStretch()

        # Checkbox Simulador
        self.chk_mock = QCheckBox("Simulador (Mock)")
        self.chk_mock.setChecked(True)  # Inicia marcado para facilitar testes rápidos
        self.chk_mock.toggled.connect(self.on_mock_toggled)
        header_layout.addWidget(self.chk_mock)

        # Dropdown de Portas
        header_layout.addWidget(QLabel("Porta:"))
        self.cmb_ports = QComboBox()
        header_layout.addWidget(self.cmb_ports)

        # Botão Atualizar Portas
        self.btn_refresh = QPushButton("Atualizar")
        self.btn_refresh.clicked.connect(self.refresh_ports)
        header_layout.addWidget(self.btn_refresh)

        # Baudrate
        header_layout.addWidget(QLabel("Baud:"))
        self.cmb_baud = QComboBox()
        self.cmb_baud.addItems(["9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"])
        self.cmb_baud.setCurrentText("115200")
        header_layout.addWidget(self.cmb_baud)

        # Botão Conectar
        self.btn_connect = QPushButton("Conectar")
        self.btn_connect.setObjectName("btnConnect")
        self.btn_connect.clicked.connect(self.toggle_connection)
        header_layout.addWidget(self.btn_connect)

        main_layout.addWidget(header_frame)

        # -------------------------------------------------------------
        # 2. TIME-LINE DOS ESTADOS DE VOO (FSM)
        # -------------------------------------------------------------
        timeline_frame = QFrame()
        timeline_frame.setObjectName("cardFrame")
        timeline_layout = QHBoxLayout(timeline_frame)
        timeline_layout.setContentsMargins(8, 4, 8, 4)
        timeline_layout.setSpacing(4)

        self.fsm_labels = {}
        for state_id, state_name in FLIGHT_STATES.items():
            lbl = QLabel(state_name)
            lbl.setAlignment(Qt.AlignCenter)
            lbl.setStyleSheet(
                "padding: 6px; font-weight: bold; border-radius: 4px; "
                "background-color: #171412; color: #57534e; border: 1px solid #2e2a24;"
            )
            lbl.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
            timeline_layout.addWidget(lbl)
            self.fsm_labels[state_id] = lbl

        main_layout.addWidget(timeline_frame)

        # -------------------------------------------------------------
        # 3. CONTEÚDO PRINCIPAL (DADOS E GRÁFICOS)
        # -------------------------------------------------------------
        body_layout = QHBoxLayout()
        body_layout.setSpacing(10)

        # 3.1 Painel de Métricas (Esquerda)
        metrics_layout = QVBoxLayout()
        metrics_layout.setSpacing(10)

        # Grid de Cards de Dados
        data_grid = QGridLayout()
        data_grid.setSpacing(10)

        # Card 1: Altura
        self.card_alt = self.create_card("ALTITUDE", "0.0 m", "card_alt")
        data_grid.addWidget(self.card_alt, 0, 0)

        # Card 2: Velocidade
        self.card_vel = self.create_card("VEL. VERTICAL", "0.0 m/s", "card_vel")
        data_grid.addWidget(self.card_vel, 0, 1)

        # Card 3: Aceleração resultante
        self.card_acc = self.create_card("ACELERAÇÃO", "1.00 g", "card_acc")
        data_grid.addWidget(self.card_acc, 1, 0)

        # Card 4: Tensão da Bateria
        self.card_bat = self.create_card("BATERIA", "0.00 V", "card_bat", green=True)
        data_grid.addWidget(self.card_bat, 1, 1)

        # Card 5: GPS Localização
        self.card_gps_pos = self.create_card("GPS COORDENADAS", "LAT: ---\nLON: ---", "card_gps_pos", font_size=13)
        data_grid.addWidget(self.card_gps_pos, 2, 0)

        # Card 6: GPS Status
        self.card_gps_status = self.create_card("GPS STATUS", "SEM SINAL (0 sats)", "card_gps_status", rose=True)
        data_grid.addWidget(self.card_gps_status, 2, 1)

        # Card 7: Link / Estatísticas
        self.card_link = self.create_card("PACOTES (LINK)", "REC: 0 | CRC: 0\nTAXA: 0.0 Hz", "card_link")
        data_grid.addWidget(self.card_link, 3, 0)

        # Card 8: Cronômetro / boot
        self.card_time = self.create_card("TEMPO DE VOO", "00:00:00.0", "card_time")
        data_grid.addWidget(self.card_time, 3, 1)

        metrics_layout.addLayout(data_grid)
        body_layout.addLayout(metrics_layout, stretch=2)

        # 3.2 Painel de Gráficos e GPS (Direita)
        plots_frame = QFrame()
        plots_frame.setObjectName("cardFrame")
        plots_layout = QVBoxLayout(plots_frame)
        plots_layout.setContentsMargins(8, 8, 8, 8)

        self.data_tabs = QTabWidget()

        flight_plots_widget = QWidget()
        flight_plots_layout = QVBoxLayout(flight_plots_widget)
        flight_plots_layout.setContentsMargins(0, 0, 0, 0)

        # Gráfico de Altitude
        self.plot_alt = pg.PlotWidget(title="Altitude Relativa (m)")
        self.plot_alt.setBackground("#171412")
        self.plot_alt.showGrid(x=True, y=True, alpha=0.2)
        self.alt_curve = self.plot_alt.plot(pen=pg.mkPen('#06b6d4', width=2), antialias=True)
        flight_plots_layout.addWidget(self.plot_alt)

        # Gráfico de Velocidade e Aceleração
        self.plot_dynamics = pg.PlotWidget(title="Velocidade (m/s) & Aceleração (g)")
        self.plot_dynamics.setBackground("#171412")
        self.plot_dynamics.showGrid(x=True, y=True, alpha=0.2)
        self.plot_dynamics.addLegend(offset=(10, 10))
        self.vel_curve = self.plot_dynamics.plot(name="Velocidade (m/s)", pen=pg.mkPen('#10b981', width=2), antialias=True)
        self.acc_curve = self.plot_dynamics.plot(name="Aceleração (g)", pen=pg.mkPen('#f43f5e', width=1.5), antialias=True)
        flight_plots_layout.addWidget(self.plot_dynamics)
        self.data_tabs.addTab(flight_plots_widget, "VOO")

        gps_widget = QWidget()
        gps_layout = QVBoxLayout(gps_widget)
        gps_layout.setContentsMargins(0, 0, 0, 0)

        gps_info_layout = QHBoxLayout()
        self.lbl_gps_details = QLabel(
            "Aguardando dados GPS...\n"
            "Latitude: --- | Longitude: ---\n"
            "Altitude: --- | Satélites: 0"
        )
        self.lbl_gps_details.setObjectName("gpsDetails")
        self.lbl_gps_details.setTextInteractionFlags(Qt.TextSelectableByMouse)
        gps_info_layout.addWidget(self.lbl_gps_details, stretch=1)

        self.btn_open_map = QPushButton("ABRIR NO MAPA")
        self.btn_open_map.setEnabled(False)
        self.btn_open_map.clicked.connect(self.open_gps_in_map)
        gps_info_layout.addWidget(self.btn_open_map)
        gps_info_layout.addSpacing(8)
        gps_layout.addLayout(gps_info_layout)

        self.plot_gps = pg.PlotWidget(title="Trajeto GPS")
        self.plot_gps.setBackground("#171412")
        self.plot_gps.showGrid(x=True, y=True, alpha=0.2)
        self.plot_gps.setLabel("bottom", "Longitude", units="graus")
        self.plot_gps.setLabel("left", "Latitude", units="graus")
        self.plot_gps.setAspectLocked(True)
        self.gps_path_curve = self.plot_gps.plot(
            pen=pg.mkPen("#f59e0b", width=2),
            symbol="o",
            symbolSize=4,
            symbolBrush="#f59e0b"
        )
        self.gps_current_marker = self.plot_gps.plot(
            pen=None,
            symbol="o",
            symbolSize=12,
            symbolBrush="#10b981",
            symbolPen=pg.mkPen("#ffffff", width=1)
        )
        gps_layout.addWidget(self.plot_gps)
        self.data_tabs.addTab(gps_widget, "GPS / TRAJETO")

        plots_layout.addWidget(self.data_tabs)

        body_layout.addWidget(plots_frame, stretch=3)
        main_layout.addLayout(body_layout, stretch=4)

        # -------------------------------------------------------------
        # 4. PAINEL DE CONTROLE (BOTÕES E LOGS)
        # -------------------------------------------------------------
        bottom_layout = QHBoxLayout()
        bottom_layout.setSpacing(10)

        # Painel de Comandos (Esquerda)
        cmd_group = QGroupBox("Comandos da Estação Base")
        cmd_grid = QGridLayout(cmd_group)
        cmd_grid.setSpacing(8)

        # Botão Desarmar
        self.btn_disarm = QPushButton("DESARMAR FOGUETE")
        self.btn_disarm.setObjectName("btnDisarm")
        self.btn_disarm.clicked.connect(lambda: self.send_command_id(0x02))
        cmd_grid.addWidget(self.btn_disarm, 0, 0)

        # Botão Ping
        self.btn_ping = QPushButton("PING (TESTE)")
        self.btn_ping.clicked.connect(lambda: self.send_command_id(0x03))
        cmd_grid.addWidget(self.btn_ping, 0, 1)

        # Botão Iniciar Gravação SD
        self.btn_start_log = QPushButton("INICIAR LOG SD")
        self.btn_start_log.clicked.connect(lambda: self.send_command_id(0x04))
        cmd_grid.addWidget(self.btn_start_log, 1, 0)

        # Botão Parar Gravação SD
        self.btn_stop_log = QPushButton("PARAR LOG SD")
        self.btn_stop_log.clicked.connect(lambda: self.send_command_id(0x05))
        cmd_grid.addWidget(self.btn_stop_log, 1, 1)

        bottom_layout.addWidget(cmd_group, stretch=2)

        # Console de Logs (Direita)
        console_group = QGroupBox("Console de Eventos e Logs")
        console_layout = QVBoxLayout(console_group)
        console_layout.setContentsMargins(6, 6, 6, 6)

        self.txt_console = QTextEdit()
        self.txt_console.setReadOnly(True)
        console_layout.addWidget(self.txt_console)

        bottom_layout.addWidget(console_group, stretch=3)
        main_layout.addLayout(bottom_layout, stretch=2)

        # Desabilita botões de comando inicialmente
        self.set_commands_enabled(False)

    def create_card(self, title: str, initial_val: str, name: str, green=False, rose=False, font_size=18) -> QFrame:
        card = QFrame()
        card.setObjectName("cardFrame")
        card.setMinimumHeight(80)
        
        layout = QVBoxLayout(card)
        layout.setContentsMargins(10, 8, 10, 8)
        layout.setSpacing(2)

        lbl_title = QLabel(title)
        lbl_title.setObjectName("lblMetricTitle")
        layout.addWidget(lbl_title)

        lbl_val = QLabel(initial_val)
        if green:
            lbl_val.setObjectName("valMonospaceGreen")
        elif rose:
            lbl_val.setObjectName("valMonospaceRose")
        else:
            lbl_val.setObjectName("valMonospace")
        
        # Ajustar tamanho da fonte para leituras mais complexas (como coordenadas)
        if font_size != 18:
            lbl_val.setStyleSheet(f"font-size: {font_size}px;")
            
        layout.addWidget(lbl_val)
        
        # Guarda referência para atualizar valor dinamicamente
        setattr(self, f"lbl_{name}_val", lbl_val)
        
        return card

    # -------------------------------------------------------------
    # GESTÃO DA CONEXÃO E PORTAS
    # -------------------------------------------------------------
    def refresh_ports(self):
        self.cmb_ports.clear()
        
        # Se estiver em modo simulado, adiciona MOCK0
        if self.chk_mock.isChecked():
            self.cmb_ports.addItem("MOCK0")
            self.cmb_ports.setEnabled(False)
            self.btn_refresh.setEnabled(False)
        else:
            self.cmb_ports.setEnabled(True)
            self.btn_refresh.setEnabled(True)
            ports = [p.device for p in serial.tools.list_ports.comports()]
            if ports:
                self.cmb_ports.addItems(ports)
            else:
                self.log("⚠️ Nenhuma porta serial física encontrada.")

    def on_mock_toggled(self, checked):
        self.refresh_ports()
        if checked:
            self.log("ℹ️ Modo simulador ativado.")
        else:
            self.log("ℹ️ Modo de porta física serial ativado.")

    def toggle_connection(self):
        if self.is_connected:
            self.disconnect_serial()
        else:
            self.connect_serial()

    def connect_serial(self):
        port = self.cmb_ports.currentText()
        if not port:
            self.log("❌ Erro: Selecione uma porta serial para conectar.")
            return

        baud = int(self.cmb_baud.currentText())
        is_mock = self.chk_mock.isChecked()

        # Limpar buffers de gráficos
        self.time_history.clear()
        self.alt_history.clear()
        self.vel_history.clear()
        self.acc_history.clear()
        self.gps_lat_history.clear()
        self.gps_lon_history.clear()
        self.last_valid_gps = None
        self.gps_path_curve.setData([], [])
        self.gps_current_marker.setData([], [])
        self.btn_open_map.setEnabled(False)

        # Iniciar worker serial
        self.serial_worker = SerialWorker(port, baud, is_mock=is_mock)
        self.serial_worker.packet_received.connect(self.on_packet_received)
        self.serial_worker.log_message.connect(self.log)
        self.serial_worker.status_changed.connect(self.on_connection_status_changed)
        self.serial_worker.start()

    def disconnect_serial(self):
        if self.serial_worker:
            self.serial_worker.running = False
            self.serial_worker.wait()
            self.serial_worker = None
        self.close_csv_logger()

    def on_connection_status_changed(self, connected: bool):
        self.is_connected = connected
        if connected:
            self.btn_connect.setText("Desconectar")
            self.btn_connect.setObjectName("btnDisconnect")
            self.set_commands_enabled(True)
            
            # Bloquear seletores
            self.chk_mock.setEnabled(False)
            self.cmb_ports.setEnabled(False)
            self.cmb_baud.setEnabled(False)
            self.btn_refresh.setEnabled(False)

            # Iniciar arquivo de telemetria CSV
            self.init_csv_logger()
        else:
            self.btn_connect.setText("Conectar")
            self.btn_connect.setObjectName("btnConnect")
            self.set_commands_enabled(False)
            
            # Liberar seletores
            self.chk_mock.setEnabled(True)
            self.on_mock_toggled(self.chk_mock.isChecked())
            self.cmb_baud.setEnabled(True)
            
            # Resetar cores da timeline
            self.update_fsm_timeline(None)
            self.close_csv_logger()

        # Recarrega folhas de estilo para atualizar as cores dos botões de conexão
        self.setStyleSheet(QSS_STYLE)

    def set_commands_enabled(self, enabled: bool):
        self.btn_disarm.setEnabled(enabled)
        self.btn_ping.setEnabled(enabled)
        self.btn_start_log.setEnabled(enabled)
        self.btn_stop_log.setEnabled(enabled)

    # -------------------------------------------------------------
    # GRAVAÇÃO DOS DADOS EM CSV
    # -------------------------------------------------------------
    def init_csv_logger(self):
        os.makedirs("logs", exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        port_clean = self.cmb_ports.currentText().replace("/", "_").replace("\\", "_")
        self.csv_filepath = f"logs/telemetria_{timestamp}_{port_clean}.csv"
        
        try:
            self.csv_file = open(self.csv_filepath, mode="w", newline="", encoding="utf-8")
            self.csv_writer = csv.writer(self.csv_file)
            # Cabeçalho do CSV
            self.csv_writer.writerow([
                "Timestamp_Local",
                "Timestamp_Boot_ms",
                "Packet_ID",
                "Altitude_m",
                "Velocity_m_s",
                "Acceleration_g",
                "Battery_mV",
                "Flight_State_ID",
                "Flight_State_Name",
                "GPS_Latitude",
                "GPS_Longitude",
                "GPS_Altitude_m",
                "GPS_Fix",
                "GPS_Satellites"
            ])
            self.log(f"💾 Gravação local iniciada em: {self.csv_filepath}")
        except Exception as e:
            self.log(f"⚠️ Falha ao criar arquivo de gravação CSV: {str(e)}")

    def write_packet_to_csv(self, p: dict):
        if self.csv_writer and self.csv_file:
            try:
                self.csv_writer.writerow([
                    datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3],
                    p["timestamp_ms"],
                    p["packet_id"],
                    f"{p['altitude_m']:.3f}",
                    f"{p['vert_velocity_ms']:.3f}",
                    f"{p['acceleration_g']:.3f}",
                    p["battery_mv"],
                    p["flight_state_id"],
                    p["flight_state"],
                    f"{p['gps_latitude']:.7f}",
                    f"{p['gps_longitude']:.7f}",
                    p["gps_altitude_m"],
                    1 if p["gps_fix"] else 0,
                    p["gps_satellites"]
                ])
                # Dá flush frequente para evitar perda de dados em crash
                self.csv_file.flush()
            except Exception as e:
                self.log(f"⚠️ Erro ao escrever dados no CSV: {str(e)}")

    def close_csv_logger(self):
        if self.csv_file:
            try:
                self.csv_file.close()
                self.log(f"💾 Arquivo de gravação CSV fechado com sucesso.")
            except:
                pass
            self.csv_file = None
            self.csv_writer = None
            self.csv_filepath = None

    # -------------------------------------------------------------
    # RECEPÇÃO E PARSE DE PACOTES
    # -------------------------------------------------------------
    def on_packet_received(self, p: dict):
        packet_type = p.get("type")

        if packet_type == "TELEMETRY":
            self.packet_count += 1
            self.packet_rate_counter += 1
            
            # Salva no arquivo CSV local
            self.write_packet_to_csv(p)

            # Atualizar widgets de métricas
            self.lbl_card_alt_val.setText(f"{p['altitude_m']:.1f} m")
            self.lbl_card_vel_val.setText(f"{p['vert_velocity_ms']:.1f} m/s")
            self.lbl_card_acc_val.setText(f"{p['acceleration_g']:.2f} g")
            self.lbl_card_bat_val.setText(f"{p['battery_v']:.2f} V")
            
            # GPS
            lat_str = f"LAT: {p['gps_latitude']:.7f}"
            lon_str = f"LON: {p['gps_longitude']:.7f}"
            self.lbl_card_gps_pos_val.setText(f"{lat_str}\n{lon_str}")
            self.lbl_gps_details.setText(
                f"{'FIX ATIVO' if p['gps_fix'] else 'SEM FIX'}\n"
                f"Latitude: {p['gps_latitude']:.7f} | Longitude: {p['gps_longitude']:.7f}\n"
                f"Altitude GPS: {p['gps_altitude_m']} m | Satélites: {p['gps_satellites']}"
            )
            
            if p["gps_fix"]:
                self.lbl_card_gps_status_val.setText(f"FIX ATIVO ({p['gps_satellites']} sats)")
                self.lbl_card_gps_status_val.setObjectName("valMonospaceGreen")
                self.update_gps_track(
                    p["gps_latitude"],
                    p["gps_longitude"],
                    p["gps_altitude_m"],
                    p["gps_satellites"]
                )
            else:
                self.lbl_card_gps_status_val.setText(f"SEM FIX ({p['gps_satellites']} sats)")
                self.lbl_card_gps_status_val.setObjectName("valMonospaceRose")
            self.lbl_card_gps_status_val.setStyleSheet("") # Força recarregamento do estilo

            # Tempo de boot do foguete (Formatado)
            boot_s = p["timestamp_ms"] / 1000.0
            h = int(boot_s // 3600)
            m = int((boot_s % 3600) // 60)
            s = boot_s % 60
            self.lbl_card_time_val.setText(f"{h:02d}:{m:02d}:{s:04.1f}")

            # FSM Timeline
            self.update_fsm_timeline(p["flight_state_id"])

            # Histórico para Gráficos
            self.time_history.append(boot_s)
            self.alt_history.append(p["altitude_m"])
            self.vel_history.append(p["vert_velocity_ms"])
            self.acc_history.append(p["acceleration_g"])

            # Limita tamanho dos vetores para performance
            if len(self.time_history) > self.max_history_len:
                self.time_history.pop(0)
                self.alt_history.pop(0)
                self.vel_history.pop(0)
                self.acc_history.pop(0)

            # Atualizar Gráficos
            self.alt_curve.setData(self.time_history, self.alt_history)
            self.vel_curve.setData(self.time_history, self.vel_history)
            self.acc_curve.setData(self.time_history, self.acc_history)

        elif packet_type == "COMMAND_RESPONSE":
            cmd_name = p.get("command")
            result = p.get("result")
            result_id = p.get("result_id")
            
            if result_id == 0:  # OK / Sucesso
                self.log(f"🟢 [RX RESPOSTA] Comando {cmd_name} executado com SUCESSO pelo foguete!")
            else:
                self.log(f"🔴 [RX RESPOSTA] Comando {cmd_name} FALHOU no foguete: {result} (Código {result_id})")

        elif packet_type == "CRC_ERROR":
            self.crc_errors += 1
            self.log(f"⚠️ [CRC ERROR] Pacote corrompido recebido (Calculado: 0x{p['expected']:04X} | Lido: 0x{p['received']:04X})")

    def update_gps_track(self, latitude: float, longitude: float, altitude_m: int, satellites: int):
        if not (-90.0 <= latitude <= 90.0 and -180.0 <= longitude <= 180.0):
            return

        self.last_valid_gps = (latitude, longitude)
        self.gps_lat_history.append(latitude)
        self.gps_lon_history.append(longitude)

        if len(self.gps_lat_history) > self.max_history_len:
            self.gps_lat_history.pop(0)
            self.gps_lon_history.pop(0)

        self.gps_path_curve.setData(self.gps_lon_history, self.gps_lat_history)
        self.gps_current_marker.setData([longitude], [latitude])
        self.btn_open_map.setEnabled(True)

    def open_gps_in_map(self):
        if self.last_valid_gps is None:
            self.log("GPS sem posição válida para abrir no mapa.")
            return

        latitude, longitude = self.last_valid_gps
        url = QUrl(
            f"https://www.openstreetmap.org/"
            f"?mlat={latitude:.7f}&mlon={longitude:.7f}"
            f"#map=17/{latitude:.7f}/{longitude:.7f}"
        )
        if not QDesktopServices.openUrl(url):
            self.log("Não foi possível abrir o navegador com a posição GPS.")

    # -------------------------------------------------------------
    # TIMELINE DE ESTADOS FSM
    # -------------------------------------------------------------
    def update_fsm_timeline(self, active_state_id: Optional[int]):
        for state_id, lbl in self.fsm_labels.items():
            if active_state_id is not None and state_id == active_state_id:
                # Destaques visuais por estado
                if state_id == 2:  # ARMED (Vermelho de atenção)
                    bg_color = "#991b1b"
                    text_color = "#ffffff"
                elif state_id in [3, 4]:  # ASCENT / APOGEE (Ciano ativo)
                    bg_color = "#0369a1"
                    text_color = "#ffffff"
                elif state_id == 5:  # DESCENT (Azul cobalto)
                    bg_color = "#1d4ed8"
                    text_color = "#ffffff"
                elif state_id == 6:  # LANDED (Verde esmeralda de sucesso)
                    bg_color = "#047857"
                    text_color = "#ffffff"
                else:  # Outros ativos
                    bg_color = "#d97706"
                    text_color = "#ffffff"
                
                lbl.setStyleSheet(
                    f"padding: 6px; font-weight: bold; border-radius: 4px; "
                    f"background-color: {bg_color}; color: {text_color}; border: 1px solid {bg_color};"
                )
            else:
                # Estados inativos
                lbl.setStyleSheet(
                    "padding: 6px; font-weight: bold; border-radius: 4px; "
                    "background-color: #171412; color: #57534e; border: 1px solid #2e2a24;"
                )

    # -------------------------------------------------------------
    # TRANSMISSÃO DE COMANDOS
    # -------------------------------------------------------------
    def send_command_id(self, cmd_id: int):
        if not self.is_connected or not self.serial_worker:
            self.log("❌ Erro: Conecte à porta serial antes de enviar comandos.")
            return

        cmd_name = {1: "ARM", 2: "DISARM", 3: "PING", 4: "START_LOG", 5: "STOP_LOG"}.get(cmd_id, str(cmd_id))
        self.log(f"💬 Solicitando envio do comando {cmd_name}...")
        self.serial_worker.queue_command(cmd_id)

    # -------------------------------------------------------------
    # AUXILIARES E TIMERS
    # -------------------------------------------------------------
    def update_second_stats(self):
        # Calcula taxa de pacotes por segundo
        self.packet_rate = self.packet_rate_counter
        self.packet_rate_counter = 0

        # Atualizar card de Link / Taxa
        if self.is_connected:
            self.lbl_card_link_val.setText(
                f"REC: {self.packet_count} | CRC: {self.crc_errors}\n"
                f"TAXA: {self.packet_rate:.1f} Hz"
            )
        else:
            self.lbl_card_link_val.setText("DESCONECTADO\nTAXA: 0.0 Hz")

    def toggle_blink(self):
        self.blink_state = not self.blink_state
        # Se estiver armado, faz piscar o rótulo ARMED
        if self.is_connected and hasattr(self, "serial_worker") and self.serial_worker:
            # Pega o último estado da FSM
            active_lbl = None
            # Verifica qual label está ativa
            # Para piscar a do ARMED (estado 2)
            lbl_armed = self.fsm_labels.get(2)
            # Se for o estado atual do rocket, fazemos piscar
            # Só pescamos do buffer da label
            # Mas para manter simples, se a label está com fundo vermelho, piscamos
            # Vamos verificar o estilo atual ou usar o valor do parser
            parser_state = self.serial_worker.parser.buffer
            # Uma alternativa mais robusta:
            # Pega o flight_state_id mais recente que conhecemos de forma simples:
            # Nós guardamos no parser
            pass

    def log(self, message: str):
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.txt_console.append(f"[{timestamp}] {message}")
        # Rola o scrollbar para o fim
        self.txt_console.verticalScrollBar().setValue(
            self.txt_console.verticalScrollBar().maximum()
        )

    def closeEvent(self, event):
        self.disconnect_serial()
        event.accept()

def main():
    app = QApplication(sys.argv)
    
    # Define fonte padrão limpa
    font = QFont("Segoe UI", 10)
    app.setFont(font)
    
    gui = BaseStationGUI()
    gui.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
