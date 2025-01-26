import serial
import matplotlib.pyplot as plt
import csv
import datetime
import time
from matplotlib.dates import DateFormatter, MinuteLocator
import numpy as np
import re  # Para usar expressões regulares

# Pergunta ao usuário quanto tempo (em segundos) deve durar a medição
tempo_medicao = int(input("Quantos segundos você quer medir a temperatura? "))

# Configuração da porta serial (ajuste conforme necessário)
ser = serial.Serial('COM5', 115200)  # Altere para a porta correta
ser.flush()

# Inicializa lista para armazenar dados de temperatura
temps = []
times = []

# Cria um timestamp para o início da medição e usa no nome do arquivo
start_timestamp = datetime.datetime.now()
nome_arquivo_base = start_timestamp.strftime(f"%Y-%m-%d_%H-%M-%S_{tempo_medicao}s")

# Define o caminho onde os arquivos serão salvos
caminho_diretorio = "G:\\Meu Drive\\UFF\\LMS\\TCC\\TemperatureSensor-Codes\\TemperatureSensor-Python"

# Configura o nome dos arquivos CSV e PNG com o caminho completo
csv_filename = f"{caminho_diretorio}/{nome_arquivo_base}.csv"
img_filename = f"{caminho_diretorio}/{nome_arquivo_base}.png"

# Cria e escreve o cabeçalho no arquivo CSV
with open(csv_filename, "w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp", "Temperatura (°C)"])  # Cabeçalho do CSV

# Configuração do gráfico
fig, ax = plt.subplots()
line, = ax.plot([], [], '-', color="blue")  # Substituindo plot_date por plot

# Define o intervalo de marcação do eixo X com base na duração da medição
if tempo_medicao < 600:  # Menor que 10 minutos
    ax.xaxis.set_major_locator(MinuteLocator(interval=1))  # Marcações a cada 1 minuto
else:
    ax.xaxis.set_major_locator(MinuteLocator(interval=5))  # Marcações a cada 5 minutos

ax.xaxis.set_major_formatter(DateFormatter("%H:%M"))  # Exibe o horário

plt.xlabel("Tempo")
plt.ylabel("Temperatura (°C)")

# Definindo a escala do eixo Y para que vá de 0°C a 50°C
ax.set_ylim(0, 50)

plt.tight_layout()

# Função de atualização do gráfico
def update_graph():
    line.set_data(times, temps)
    ax.relim()
    ax.autoscale_view()  # Ajusta a escala do gráfico conforme necessário
    plt.draw()  # Redesenha o gráfico

# Inicia a medição por tempo determinado
start_time = time.time()
while time.time() - start_time < tempo_medicao:
    if ser.in_waiting > 0:
        line_data = ser.readline().decode('utf-8').strip()

        # Verifica se a linha contém a palavra 'Temperature' e segue o formato esperado
        if "Temperature" in line_data:
            # Usando regex para extrair o número da temperatura
            match = re.search(r"Temperature\s*=\s*([-\d.]+)\s*\*C", line_data)
            if match:
                tempC = float(match.group(1))  # Extrai e converte o valor da temperatura
                timestamp = datetime.datetime.now()

                # Armazena dados na lista
                temps.append(tempC)
                times.append(timestamp)

                # Grava dados no CSV a cada leitura
                with open(csv_filename, "a", newline="") as file:
                    writer = csv.writer(file)
                    writer.writerow([timestamp.strftime('%Y-%m-%d %H:%M:%S'), tempC])

                # Atualiza o gráfico a cada leitura
                update_graph()
            else:
                print(f"Ignorando dado inválido: {line_data}")

    time.sleep(0.1)  # Aguarda 100ms para a próxima leitura

# Após a coleta, calcula a média de todas as medições
if temps:
    media_temp = np.mean(temps)
    # Adiciona a linha da média
    ax.axhline(y=media_temp, color='red', linestyle='--',linewidth=0.5, label=f'Média: {media_temp:.2f}°C')

# Exibe a legenda e o gráfico após o período de medição
plt.legend(loc='upper left')  # Adiciona a legenda para a linha da média

# Salva a imagem do gráfico no caminho especificado
plt.savefig(img_filename)  # Salva o gráfico como "YYYY-MM-DD_HH-MM-SS_duracao.png"

plt.show()
