# Aturan untuk isi sel
<!-- keywords: validasi, aturan, validasi data, daftar, daftar turun-bawah, drop-down, dropdown, bilangan bulat, desimal, tanggal, waktu, panjang, kustom, henti, peringatan, informasi, peringatan galat, tidak diizinkan, pilihan, panah, Alt+Bawah -->

Buku kerja dapat memiliki **aturan** yang menentukan isi sel tertentu: bilangan bulat dari 1 sampai 10, tanggal setelah hari tertentu, teks dengan paling banyak lima huruf, atau satu pilihan dari sebuah daftar. Aturan berasal dari buku kerja; JeffCrunch menerapkannya saat Anda **mengetik** nilai ke dalam sel (atau memilih satu dari daftarnya). Nilai yang Anda tempel, isi, atau pindahkan tidak diperiksa, seperti pada program lembar sebar yang sudah umum, sehingga aturan hanya menentukan apa yang boleh diketik, bukan apa yang tersimpan di dalam sel.

## Saat nilai tidak sesuai

Setiap aturan memiliki **gaya**, dan pesannya adalah pesan milik aturan itu jika buku kerja memilikinya:

- **Henti**: pesan muncul dan entri tetap berada di dalam sel agar Anda dapat mengubahnya. Tekan **Esc** untuk membatalkannya.
- **Peringatan**: Anda ditanya apakah tetap ingin menyimpan nilai tersebut.
- **Informasi**: Anda diberi tahu bahwa nilai tidak sesuai, dan dapat melanjutkan atau membatalkannya.

Nilai yang Anda tempel tidak diperiksa terhadap aturan: nilai itu dimasukkan apa adanya, dan tidak ada pertanyaan yang diajukan. Menghapus isi sel selalu diizinkan.

## Pilih dari daftar

Sel yang memiliki daftar menampilkan tombol kecil dengan panah di tepi kanannya saat disorot kursor. Klik tombol tersebut, atau tekan **Alt+Bawah**, untuk melihat pilihan, lalu klik salah satunya untuk memasukkannya.

## Yang tidak dilakukan

JeffCrunch menyimpan aturan yang dimiliki buku kerja, tetapi belum memiliki perintah untuk membuat aturan baru. Aturan yang mengambil pilihannya dari lembar lain dengan cara penulisan yang lebih baru disimpan di dalam berkas, tetapi tidak diterapkan. Pesan yang muncul saat Anda memilih sel tidak ditampilkan.
