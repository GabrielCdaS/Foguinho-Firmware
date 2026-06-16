# Estação de Base - Visualizador e Controlador em Tempo Real 🚀

Este diretório contém uma aplicação desktop desenvolvida em Python 3 utilizando **PySide6** (Qt6) e **pyqtgraph** para monitoramento em tempo real da telemetria do foguete Foguinho e envio de comandos de controle.

## Recursos Principais
- **Conexão Flexível**: Suporta portas seriais físicas (baseadas em USB CDC virtual COM) ou **Modo Simulador** integrado para demonstrar e testar o voo e comandos sem hardware.
- **Linha do Tempo FSM**: Acompanhamento visual e colorido da fase atual de voo (BOOT, IDLE, ARMED, ASCENT, APOGEE, DESCENT, LANDED).
- **Visualização GPS**: Coordenadas, altitude GPS, fix, satélites e trajeto em tempo real, com botão para abrir a posição atual no OpenStreetMap.
- **Gráficos em Tempo Real**: Curvas de altitude, velocidade e aceleração dinâmicas e de alta performance.
- **Painel de Controle**: Envio de comandos de armamento (`ARM`/`DISARM`), conectividade (`PING`) e datalogger (`START LOG`/`STOP LOG`).
- **Logs CSV**: Gravação automática de toda a telemetria recebida no diretório local `logs/` com carimbo de data/hora.

## Como Executar

### 1. Criar um Ambiente Virtual
Para manter as dependências isoladas, crie um ambiente virtual na pasta do programa:
```powershell
python -m venv .venv
```

### 2. Ativar o Ambiente Virtual
No Windows PowerShell, execute:
```powershell
.venv\Scripts\Activate.ps1
```
*(Se encontrar erro de permissão do PowerShell, execute `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process` antes de ativar)*

### 3. Instalar Dependências
Com o ambiente ativado, instale as bibliotecas necessárias listadas em `requirements.txt`:
```powershell
pip install -r requirements.txt
```

### 4. Executar a Aplicação
Inicie o dashboard executando:
```powershell
python main.py
```

## Como Usar o Modo Simulador (Sem Hardware)
1. Marque a caixa **Simulador (Mock)** no topo superior direito.
2. A porta será automaticamente alterada para `MOCK0`.
3. Clique em **Conectar**.
4. O simulador iniciará no estado `IDLE`, enviando telemetria a 10 Hz.
5. Clique em **💥 ARMAR RECUPERAÇÃO** no painel inferior. O foguete passará para o estado `ARMED` e, após 3 segundos, decolará automaticamente, executando toda a sequência de voo (Subida -> Apogeu -> Ejeção e Descida com Paraquedas -> Pouso).
