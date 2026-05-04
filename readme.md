# Penjelasan Kode Program

Program ini menggunakan konsep RTOS (Real-Time Operating System) dengan beberapa task yang bekerja secara bersamaan dan saling berkomunikasi menggunakan queue.

## 1. Library yang Digunakan

```cpp
#include <Arduino_FreeRTOS.h>
#include <queue.h>
```

Arduino_FreeRTOS.h digunakan untuk menjalankan sistem RTOS di Arduino, sedangkan queue.h digunakan untuk komunikasi data antar task.

## 2. Konfigurasi Pin

```cpp
#define LED_MERAH 13
#define LED_HIJAU 12
#define SENSOR_PIN A0
```

Pin 13 digunakan untuk LED merah yang berfungsi sebagai indikator kondisi tinggi. Pin 12 digunakan untuk LED hijau yang berfungsi sebagai indikator kondisi normal. Sedangkan A0 adalah input dari potensiometer yang akan memberikan nilai analog ke sistem.

## 3. Deklarasi Queue

```cpp
QueueHandle_t qSensor;
QueueHandle_t qFilter;
QueueHandle_t qDecision;
```

Queue digunakan untuk mengirim data antar task secara aman. qSensor digunakan untuk mengirim data dari Task Sensor ke Task Filter. qFilter digunakan untuk mengirim data dari Task Filter ke Task Decision. qDecision digunakan untuk mengirim data dari Task Decision ke Task Display.

## 4. Task Sensor

```cpp
void TaskSensor(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  int sensorValue;

  while (1) {
    sensorValue = analogRead(SENSOR_PIN);
    Serial.print("[TaskSensor]   ADC : ");
    Serial.println(sensorValue);
    
    xQueueSend(qSensor, &sensorValue, portMAX_DELAY);
    
    vTaskDelayUntil(&xLastWakeTime, 300 / portTICK_PERIOD_MS);
  }
}
```

Task Sensor membaca nilai analog dari potensiometer dengan range 0 hingga 1023 dan kemudian mengirimnya ke queue qSensor. Nilai yang dibaca ditampilkan ke Serial Monitor agar dapat dipantau. Fungsi vTaskDelayUntil digunakan untuk membuat task berjalan secara periodik setiap 300 milidetik, dan lebih stabil dibandingkan menggunakan delay biasa.

## 5. Task Filter

```cpp
void TaskFilter(void *pvParameters) {
  int data;
  int filtered;

  while (1) {
    if (xQueueReceive(qSensor, &data, portMAX_DELAY) == pdTRUE) {
      
      filtered = data;
      
      Serial.print("[TaskFilter]   Value : ");
      Serial.println(filtered);
      
      xQueueSend(qFilter, &filtered, portMAX_DELAY);
    }
  }
}
```

Task Filter menerima data dari queue qSensor dan memproses data tersebut. Pada implementasi saat ini, data diteruskan tanpa perubahan dan berfungsi sebagai perantara atau pipeline dalam sistem. Data yang sudah diproses kemudian dikirim ke queue qFilter dan ditampilkan ke Serial Monitor.

## 6. Task Decision

```cpp
void TaskDecision(void *pvParameters) {
  int data;
  int status;

  while (1) {
    if (xQueueReceive(qFilter, &data, portMAX_DELAY) == pdTRUE) {
      
      if (data > 400) {
        status = 1;
      } else {
        status = 0;
      }
      
      Serial.print("[TaskDecision] Status Code : ");
      Serial.print(status);
      if (status == 1) Serial.println(" (TINGGI)");
      else Serial.println(" (NORMAL)");
      
      xQueueSend(qDecision, &status, portMAX_DELAY);
    }
  }
}
```

Task Decision menerima data dari queue qFilter dan membuat keputusan berdasarkan nilai threshold 400. Jika nilai sensor lebih dari 400, maka status akan diset menjadi 1 yang menunjukkan kondisi tinggi. Jika nilai sensor kurang dari atau sama dengan 400, maka status akan diset menjadi 0 yang menunjukkan kondisi normal. Hasil keputusan ini kemudian dikirim ke queue qDecision dan ditampilkan ke Serial Monitor.

## 7. Task Display

```cpp
void TaskDisplay(void *pvParameters) {
  int status;

  while (1) {
    if (xQueueReceive(qDecision, &status, portMAX_DELAY) == pdTRUE) {
      
      Serial.print("[TaskDisplay]  Output : ");
      
      if (status == 1) {
        digitalWrite(LED_MERAH, HIGH);
        digitalWrite(LED_HIJAU, LOW);
        Serial.println("MERAH");
      } else {
        digitalWrite(LED_MERAH, LOW);
        digitalWrite(LED_HIJAU, HIGH);
        Serial.println("HIJAU");
      }
      
      Serial.println("--------------------------------------------------");
    }
  }
}
```

Task Display menerima status dari queue qDecision dan menampilkan hasil keputusan melalui LED. Ketika status bernilai 1, LED merah akan menyala dan LED hijau akan mati, menunjukkan kondisi tinggi. Ketika status bernilai 0, LED merah akan mati dan LED hijau akan menyala, menunjukkan kondisi normal. Task ini juga menampilkan warna LED yang sedang menyala ke Serial Monitor.

## 8. Setup

```cpp
void setup() {
  Serial.begin(9600);

  pinMode(LED_MERAH, OUTPUT);
  pinMode(LED_HIJAU, OUTPUT);

  qSensor   = xQueueCreate(5, sizeof(int));
  qFilter   = xQueueCreate(5, sizeof(int));
  qDecision = xQueueCreate(5, sizeof(int));

  xTaskCreate(TaskSensor,   "Sensor",   128, NULL, 3, NULL);
  xTaskCreate(TaskFilter,   "Filter",   128, NULL, 2, NULL);
  xTaskCreate(TaskDecision, "Decision", 128, NULL, 2, NULL);
  xTaskCreate(TaskDisplay,  "Display",  128, NULL, 1, NULL);
}
```

Fungsi setup() melakukan inisialisasi sistem di awal program. Serial.begin(9600) mengaktifkan komunikasi serial untuk keperluan debugging dan monitoring. Kedua pin LED diatur sebagai output menggunakan pinMode(). Kemudian, tiga queue dibuat dengan kapasitas 5 data bertipe integer untuk memfasilitasi komunikasi antar task. Setelah itu, keempat task RTOS dibuat dengan prioritas yang berbeda. TaskSensor memiliki prioritas 3 (tertinggi), TaskFilter dan TaskDecision memiliki prioritas 2, dan TaskDisplay memiliki prioritas 1 (terendah). Stack size untuk setiap task diset sebesar 128 bytes.

## 9. Loop

```cpp
void loop() {
  // kosong
}
```

Fungsi loop() dibiarkan kosong karena semua proses dalam program dijalankan oleh kernel RTOS. Scheduler RTOS mengatur kapan setiap task berjalan berdasarkan prioritas dan state task, sehingga tidak perlu ada kode di dalam fungsi loop().
