#include <Arduino_FreeRTOS.h>
#include <queue.h>

// ================= KONFIGURASI PIN =================
// LED indikator
#define LED_MERAH 13   // LED untuk kondisi TINGGI
#define LED_HIJAU 12   // LED untuk kondisi NORMAL

// Input sensor analog (potensiometer)
#define SENSOR_PIN A0  // Membaca nilai analog (0 - 1023)

// ================= DEKLARASI QUEUE =================
// Digunakan untuk komunikasi antar task
QueueHandle_t qSensor;    // Sensor → Filter
QueueHandle_t qFilter;    // Filter → Decision
QueueHandle_t qDecision;  // Decision → Display

// ================= TASK 1: SENSOR =================
// Membaca nilai dari potensiometer secara periodik
void TaskSensor(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount(); // untuk delay presisi
  int sensorValue;

  while (1) {
    sensorValue = analogRead(SENSOR_PIN); // baca nilai analog

    // Tampilkan nilai sensor ke Serial Monitor
    Serial.print("[TaskSensor]   ADC : ");
    Serial.println(sensorValue);

    // Kirim data ke task berikutnya melalui queue
    xQueueSend(qSensor, &sensorValue, portMAX_DELAY);

    // Delay periodik 300 ms (sinkron RTOS)
    vTaskDelayUntil(&xLastWakeTime, 300 / portTICK_PERIOD_MS);
  }
}

// ================= TASK 2: FILTER =================
// Mengolah data dari sensor (tanpa filtering tambahan)
void TaskFilter(void *pvParameters) {
  int data;
  int filtered;

  while (1) {
    // Ambil data dari queue sensor
    if (xQueueReceive(qSensor, &data, portMAX_DELAY) == pdTRUE) {

      filtered = data;  // langsung gunakan data (tanpa smoothing)

      // Tampilkan hasil proses
      Serial.print("[TaskFilter]   Value : ");
      Serial.println(filtered);

      // Kirim ke task decision
      xQueueSend(qFilter, &filtered, portMAX_DELAY);
    }
  }
}

// ================= TASK 3: DECISION =================
// Menentukan kondisi berdasarkan nilai sensor
void TaskDecision(void *pvParameters) {
  int data;
  int status;

  while (1) {
    // Ambil data dari queue filter
    if (xQueueReceive(qFilter, &data, portMAX_DELAY) == pdTRUE) {

      // Logika keputusan berdasarkan threshold
      if (data > 400) {
        status = 1; // kondisi TINGGI
      } else {
        status = 0; // kondisi NORMAL
      }

      // Tampilkan hasil keputusan
      Serial.print("[TaskDecision] Status Code : ");
      Serial.print(status);
      if (status == 1) Serial.println(" (TINGGI)");
      else Serial.println(" (NORMAL)");

      // Kirim hasil ke task display
      xQueueSend(qDecision, &status, portMAX_DELAY);
    }
  }
}

// ================= TASK 4: DISPLAY =================
// Mengontrol LED berdasarkan hasil keputusan
void TaskDisplay(void *pvParameters) {
  int status;

  while (1) {
    // Ambil status dari queue decision
    if (xQueueReceive(qDecision, &status, portMAX_DELAY) == pdTRUE) {

      Serial.print("[TaskDisplay]  Output : ");

      // Kontrol LED sesuai status
      if (status == 1) {
        digitalWrite(LED_MERAH, HIGH); // LED merah nyala
        digitalWrite(LED_HIJAU, LOW);  // LED hijau mati
        Serial.println("MERAH");
      } else {
        digitalWrite(LED_MERAH, LOW);  // LED merah mati
        digitalWrite(LED_HIJAU, HIGH); // LED hijau nyala
        Serial.println("HIJAU");
      }

      Serial.println("--------------------------------------------------");
    }
  }
}

// ================= SETUP =================
// Inisialisasi awal sistem
void setup() {
  Serial.begin(9600);

  // Set pin LED sebagai output
  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);

  // Inisialisasi queue (kapasitas 5 data bertipe int)
  qSensor   = xQueueCreate(5, sizeof(int));
  qFilter   = xQueueCreate(5, sizeof(int));
  qDecision = xQueueCreate(5, sizeof(int));

  // Membuat task RTOS
  xTaskCreate(TaskSensor, "Sensor", 128, NULL, 3, NULL);
  xTaskCreate(TaskFilter, "Filter", 128, NULL, 2, NULL);
  xTaskCreate(TaskDecision, "Decision", 128, NULL, 2, NULL);
  xTaskCreate(TaskDisplay, "Display", 128, NULL, 1, NULL);
}

// ================= LOOP =================
// Tidak digunakan karena sistem berjalan dengan RTOS
void loop() {
  // kosong
}
