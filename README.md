# SISOP-3-2026-IT-095
## Identitas

* Nama: Nur Rizki Syahbana
* NRP: 095

# Soal 1 - Present Day, Present Time

## Deskripsi

Pada soal ini diminta membuay sistem chat berbasis **socket programming** dengan **client-server**.

* **wired.c** sebagai server utama (The Wired).
* **navi.c* sebagai client (NAVI).
* **protocol.h** menyimpan port dan jumlah maksimum client.
* Seluruh komunikasi menggunakan **TCP socket**.
* Server mendukung multi-client dengan `select()` agar dapat menangani banyak koneksi secara bersamaan.

## Penjelasan Program

## 1. protocol.h

```c
#define PORT 8080
#define MAX 100
```

### Fungsi:

* `PORT` → port server yang digunakan client untuk koneksi.
* `MAX` → jumlah maksimal client.

---

## 2. wired.c (Server)

Server bertugas menerima client, menyimpan username, broadcast pesan, admin RPC, dan logging.

### Variabel

```c
int clients[MAX];
char usernames[MAX][50];
time_t start_time;
```

### Penjelasan:

* `clients[]` menyimpan socket client yang aktif.
* `usernames[][]` menyimpan nama masing-masing client.
* `start_time` mencatat waktu server saat pertama dijalankan.

---

### Fungsi `log_event()`

```c
void log_event(const char *type, const char *msg)
```

Menulis aktivitas ke file `history.log` dengan timestamp.

Contoh output:

```txt
[2026-04-29 19:00:00] [System] [SERVER ONLINE]
```

---

### Fungsi `name_exists()`

Berfungsi untuj mengecek apakah username sudah dipakai client lain.

Jika ada nama sama maka otomatis ditolak.

---

### Fungsi `broadcast()`

Mengirim pesan dari satu client ke client lain.

```c
broadcast(msg, sender);
```

---

### Server

Server memakai `select()` agar bisa:

menerima client baru
membaca pesan dari banyak client
mendeteksi disconnect
tetap berjalan tanpa blocking

```c
select(max + 1, &readfds, NULL, NULL, NULL);
```

---

### Validasi Username

Saat client pertama kali connect:

* jika nama sudah dipakai:

```txt
[System] Name already exists
```

* jika valid:

```txt
--- Welcome to The Wired, alice ---
```

---

### Broadcast Chat

Jika user mengirim:

```txt
hello semua
```

Maka user lain menerima dan bisa melihat:

```txt
[alice]: hello semua
```

---

### Admin (The Knights)

Jika username:

```txt
The Knights
```

Maka bisa kirim command:

admin 1 :  jumlah user aktif 
admin 2 :  uptime server     
admin 3 : shutdown server   

---

## 3. navi.c (Client)

Client digunakan user untuk masuk ke server dan chat.

### Alur Program

1. Membuat socket TCP.
2. Connect ke server `127.0.0.1:8080`
3. Input username.
4. Membuat thread penerima pesan.
5. Main thread mengirim pesan.

---

### Thread Receiver (Penerima)

```c
pthread_create(&tid, NULL, recv_handler, NULL);
```

Thread ini terus menerima pesan dari server.

---

### Fungsi `recv_handler()`

Jika ada pesan broadcast dari server maka langsung ditampilkan ke terminal.

Contoh:

```txt
[alice]: halo
```

---

## Cara Menjalankan

## Compile

```bash
make
```

## Jalankan Server

```bash
./wired
```

## Jalankan Client

Terminal lain:

```bash
./navi
```

---

## Contoh Penggunaan

### Client 1

```txt
Enter your name: alice
--- Welcome to The Wired, alice ---
```

### Client 2

```txt
Enter your name: lain
--- Welcome to The Wired, lain ---
```

### Chat

```txt
alice: hello
lain menerima -> [alice]: hello
```

---

## Logging

Semua aktivitas tersimpan di:

```bash
history.log
```

Contoh:

```txt
[2026-04-29 19:00:00] [System] [SERVER ONLINE]
[2026-04-29 19:00:10] [System] [User 'alice' connected]
[2026-04-29 19:00:20] [User] [[alice]: hello]
```

---

## Kendala / Kekurangan Program

Berdasarkan source code saat ini:

1. Belum ada password admin.
2. Command `/exit` belum dibuat.
3. Shutdown belum terbroadcast ke client lain.
4. Username `The Knights` langsung auto admin tanpa autentikasi.

Walaupun belum 100% sesuai spesifikasi soal, fondasi utama komunikasi jaringan sudah berjalan dengan baik.
