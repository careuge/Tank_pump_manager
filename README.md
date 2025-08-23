# Controller per Serbatoio d'Acqua con ESP8266

## 1. Panoramica del Progetto

Questo firmware trasforma un ESP8266 in un controller intelligente per un serbatoio d'acqua. È progettato per gestire una pompa, monitorare il livello dell'acqua e il volume erogato, e offrire pieno controllo e monitoraggio da remoto tramite MQTT.

## 2. Caratteristiche Principali

- **Connettività WiFi e MQTT**: Si connette alla rete locale e a un broker MQTT.
- **Controllo Pompa Flessibile**:
  - Manuale (continuo)
  - A tempo (eroga per un numero di secondi definito)
  - A volume (eroga un numero di litri definito)
- **Monitoraggio Sensori**:
  - Livello dell'acqua tramite sensore a ultrasuoni (es. HC-SR04).
  - Flusso d'acqua tramite misuratore di flusso (es. YF-S201).
- **Calcolo Volume Serbatoio**: Calcola automaticamente il volume d'acqua in un serbatoio cilindrico orizzontale.
- **Logica di Sicurezza**: Arresta automaticamente la pompa se il livello dell'acqua scende sotto una soglia minima e ne impedisce il riavvio finché il livello non risale a sufficienza.
- **Configurazione Remota**: Tutti i parametri operativi (soglie, dimensioni del serbatoio, credenziali di rete) possono essere aggiornati da remoto via MQTT.
- **Persistenza**: Le impostazioni vengono salvate in modo permanente nella memoria EEPROM del dispositivo.
- **Funzionamento Autonomo**: Il dispositivo è in grado di operare anche in assenza di connessione di rete, utilizzando l'ultima configurazione salvata.
- **Fallback su Perdita di Connessione**: Attiva la pompa in modalità manuale in caso di perdita della connessione di rete.
- **Factory Reset**: Un pulsante fisico permette di ripristinare le impostazioni di fabbrica.

## 3. Requisiti Hardware

-   Scheda di sviluppo ESP8266 (es. NodeMCU, Wemos D1 Mini).
-   Sensore di distanza a ultrasuoni. Il modello impermeabile AJ-SR04M è consigliato e pienamente compatibile.
-   Misuratore di flusso d'acqua (es. YF-S201).
-   Modulo relè per controllare la pompa.
-   Una pompa per l'acqua.
-   Un pulsante per il reset.
-   Alimentatore adeguato per l'ESP8266 e la pompa.

Le connessioni dei pin di default sono definite in `src/Config.cpp` e possono essere modificate via MQTT.

## 4. Installazione del Firmware

Questo progetto è configurato per essere compilato con [PlatformIO](https://platformio.org/).

1.  Installa Visual Studio Code e l'estensione PlatformIO.
2.  Clona questo repository.
3.  Apri la cartella del progetto in Visual Studio Code.
4.  Modifica le credenziali WiFi di default in `src/Config.cpp` per il primo avvio.
5.  Collega la scheda ESP8266 e usa il comando "PlatformIO: Upload" per caricare il firmware.

## 5. API MQTT

Il firmware comunica usando un topic di base che può essere configurato via MQTT (default: `home/watertank`).

**Topic di Comando (`.../command/...`)**

-   `{base_topic}/command/pump`
    -   `ON`: Avvia la pompa in modalità manuale/continua.
    -   `OFF`: Ferma la pompa.
-   `{base_topic}/command/pump_duration_s`
    -   Payload (int): Avvia la pompa per il numero di secondi specificato. Es: `30`.
-   `{base_topic}/command/pump_volume_l`
    -   Payload (float): Avvia la pompa per erogare il numero di litri specificato. Es: `15.5`.

**Topic di Configurazione (`.../config/...`)**

Per aggiornare una configurazione, pubblica il nuovo valore sul topic corrispondente.

-   `{base_topic}/config/wifiSsid`: (string) SSID della rete WiFi.
-   `{base_topic}/config/wifiPassword`: (string) Password della rete WiFi.
-   `{base_topic}/config/mqttServer`: (string) Indirizzo del broker MQTT.
-   `{base_topic}/config/mqttPort`: (int) Porta del broker MQTT.
-   `{base_topic}/config/mqttUser`: (string) Utente per il broker MQTT.
-   `{base_topic}/config/mqttPassword`: (string) Password per il broker MQTT.
-   `{base_topic}/config/tankRadius`: (float) Raggio del serbatoio in metri.
-   `{base_topic}/config/tankLength`: (float) Lunghezza del serbatoio in metri.
-   `{base_topic}/config/minWaterLevel`: (float) Livello minimo dell'acqua in metri per l'arresto di sicurezza.
-   `{base_topic}/config/restartWaterLevel`: (float) Livello dell'acqua in metri necessario per poter riavviare la pompa.
-   `{base_topic}/config/sensorMountHeight`: (float) Altezza del sensore a ultrasuoni dal fondo del serbatoio, in metri.

**Topic di Stato (`.../status/...`)**

Il firmware pubblica periodicamente il suo stato su questi topic.

-   `{base_topic}/status/pump`: (string, retained) Lo stato attuale della pompa (es. `IDLE`, `PUMPING_FOR_TIME`, `STOPPED_LOW_LEVEL`).
-   `{base_topic}/status/water_level_m`: (float) Livello attuale dell'acqua in metri.
-   `{base_topic}/status/water_volume_m3`: (float) Volume attuale dell'acqua in metri cubi.
-   `{base_topic}/status/water_volume_liters`: (float, retained) Volume attuale dell'acqua in litri.
-   `{base_topic}/status/flow_rate_lpm`: (float) Portata attuale in litri al minuto.
-   `{base_topic}/status/ip_address`: (string) Indirizzo IP del dispositivo.
-   `{base_topic}/status/config`: (string) `SAVED` dopo un aggiornamento di configurazione.

## 6. Funzionamento

### Sicurezza
La priorità assoluta è la sicurezza. Il `PumpController` controlla il livello dell'acqua ad ogni ciclo del loop. Se il livello scende sotto `minWaterLevel`, la pompa viene immediatamente spenta, indipendentemente dall'operazione in corso.

### Factory Reset
Per ripristinare le impostazioni di fabbrica, tenere premuto il pulsante collegato al pin `D0` (o al pin configurato) per 10 secondi. Il dispositivo si riavvierà con le impostazioni predefinite.
