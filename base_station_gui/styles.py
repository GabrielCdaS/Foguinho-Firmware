# Estilos QSS para a interface gráfica da Estação Base (Tema Dark Premium)

QSS_STYLE = """
/* Geral */
QWidget {
    background-color: #0c0a09;
    color: #e7e5e4;
    font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, Helvetica, sans-serif;
    font-size: 13px;
}

QMainWindow {
    background-color: #0c0a09;
}

/* Painéis e Cards */
QFrame#cardFrame {
    background-color: #1c1917;
    border: 1px solid #2e2a24;
    border-radius: 8px;
}

QFrame#cardFrame:hover {
    border: 1px solid #443e34;
}

/* Painel de Controle e Abas */
QGroupBox {
    font-weight: bold;
    font-size: 12px;
    text-transform: uppercase;
    color: #a8a29e;
    border: 1px solid #2e2a24;
    border-radius: 8px;
    margin-top: 12px;
    padding-top: 16px;
    background-color: #171412;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 12px;
    padding: 0 5px;
    background-color: #0c0a09;
}

/* Botões */
QPushButton {
    background-color: #292524;
    border: 1px solid #44403c;
    border-radius: 4px;
    color: #fafaf9;
    padding: 6px 12px;
    font-weight: 500;
}

QPushButton:hover {
    background-color: #44403c;
    border: 1px solid #57534e;
}

QPushButton:pressed {
    background-color: #1c1917;
}

QPushButton:disabled {
    background-color: #1c1917;
    border: 1px solid #292524;
    color: #78716c;
}

/* Botões com Destaque Especial */
QPushButton#btnConnect {
    background-color: #0f766e;
    border: 1px solid #14b8a6;
    color: #ffffff;
    font-weight: bold;
}

QPushButton#btnConnect:hover {
    background-color: #115e59;
}

QPushButton#btnDisconnect {
    background-color: #991b1b;
    border: 1px solid #ef4444;
    color: #ffffff;
    font-weight: bold;
}

QPushButton#btnDisconnect:hover {
    background-color: #7f1d1d;
}

/* Botões de Comando Foguete */
QPushButton#btnDisarm {
    background-color: #991b1b;
    border: 1px solid #ef4444;
    color: #ffffff;
    font-weight: bold;
}

QPushButton#btnDisarm:hover {
    background-color: #7f1d1d;
}

/* ComboBox e Inputs */
QComboBox {
    background-color: #1c1917;
    border: 1px solid #44403c;
    border-radius: 4px;
    padding: 4px 8px;
    color: #fafaf9;
    min-width: 100px;
}

QComboBox:hover {
    border: 1px solid #57534e;
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 20px;
    border-left-width: 0px;
}

QComboBox QAbstractItemView {
    background-color: #1c1917;
    border: 1px solid #44403c;
    selection-background-color: #44403c;
    color: #fafaf9;
}

QCheckBox {
    color: #d6d3d1;
}

QCheckBox::indicator {
    width: 16px;
    height: 16px;
}

/* Console / Logs */
QTextEdit {
    background-color: #09090b;
    border: 1px solid #27272a;
    border-radius: 6px;
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 12px;
    color: #a7f3d0;
    padding: 8px;
}

/* Rótulos e Estatísticas */
QLabel#valMonospace {
    font-family: 'Consolas', 'Courier New', monospace;
    font-weight: bold;
    font-size: 18px;
    color: #06b6d4; /* Ciano Neon */
}

QLabel#valMonospaceGreen {
    font-family: 'Consolas', 'Courier New', monospace;
    font-weight: bold;
    font-size: 18px;
    color: #10b981; /* Esmeralda Neon */
}

QLabel#valMonospaceRose {
    font-family: 'Consolas', 'Courier New', monospace;
    font-weight: bold;
    font-size: 18px;
    color: #f43f5e; /* Rose Neon */
}

QLabel#lblMetricTitle {
    color: #a8a29e;
    font-size: 10px;
    font-weight: bold;
    text-transform: uppercase;
    letter-spacing: 1px;
}

QLabel#gpsDetails {
    font-family: 'Consolas', 'Courier New', monospace;
    color: #d6d3d1;
    padding: 6px;
}

QTabWidget::pane {
    border: 1px solid #2e2a24;
    border-radius: 4px;
    background-color: #171412;
}

QTabBar::tab {
    background-color: #1c1917;
    color: #a8a29e;
    border: 1px solid #2e2a24;
    padding: 7px 16px;
}

QTabBar::tab:selected {
    background-color: #292524;
    color: #f59e0b;
    border-bottom-color: #f59e0b;
}

/* Indicador de Status do Rádio (RSSI/SNR) */
QProgressBar {
    border: 1px solid #44403c;
    border-radius: 4px;
    text-align: center;
    background-color: #1c1917;
    color: #ffffff;
    font-weight: bold;
}

QProgressBar::chunk {
    background-color: #0f766e;
    width: 4px;
    margin: 0.5px;
}

/* Barra de Rolagem */
QScrollBar:vertical {
    border: none;
    background: #1c1917;
    width: 8px;
    margin: 0px 0 0px 0;
}

QScrollBar::handle:vertical {
    background: #44403c;
    min-height: 20px;
    border-radius: 4px;
}

QScrollBar::handle:vertical:hover {
    background: #57534e;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    border: none;
    background: none;
}
"""
