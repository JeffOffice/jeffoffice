# Persamaan
<!-- keywords: tab persamaan, struktur, simbol, konversi, persamaan, rumus, matematika, formula, pecahan, akar kuadrat, pangkat, superskrip, subskrip, eksponen, jumlah, integral, yunani, simbol, sisipkan persamaan, aksen, matriks, kurung, persamaan bawaan, galeri, ubah persamaan, edit persamaan, ods, gambar, pdf -->

Sebuah **persamaan** adalah satu baris matematika, misalnya pecahan atau akar kuadrat. Di JeffCrunch, persamaan adalah objek di atas sel, seperti gambar atau kotak teks. Persamaan bukan sel: persamaan tidak mengubah angka apa pun di lembar.

## Menyisipkan persamaan ke lembar

1. Klik **Sisipkan > Simbol > Persamaan**.
2. Ketik persamaan dalam satu baris. Di bawah kotak, Anda melihat tampilannya nanti. Klik **OK**.

Persamaan disisipkan di sel tempat Anda berada, dan dalam keadaan terpilih. Untuk memulai dari persamaan yang sudah dikenal, klik **Persamaan Bawaan** lalu pilih salah satu, misalnya rumus kuadrat atau luas lingkaran.

## Cara mengetik

- **Pecahan:** ketik garis miring. `a/b` berarti a per b. `(x+1)/(x-1)` menjadikan setiap kelompok dalam kurung sebagai satu bagian.
- **Pangkat dan subskrip:** `x^2` berarti x kuadrat, dan `x_i` berarti x dengan i kecil di bawahnya.
- **Akar:** ketik `sqrt(x+1)` untuk akar kuadrat.
- **Jumlah, integral, dan perkalian:** ketik `\sum_(i=1)^n i`, `\int_0^1 x`, atau `\prod_(k=1)^n k`.
- **Simbol:** ketik garis miring terbalik dan sebuah nama. `\alpha` adalah α, `\pi` adalah π, `\infty` adalah ∞, dan `\leq` adalah ≤.
- **Aksen, kurung, dan matriks:** `\hat{x}`, `\left( ... \right)`, dan `\pmatrix{a & b \\ c & d}`.

## Mengetik dalam UnicodeMath atau LaTeX

Di samping kotak terdapat daftar format pengetikan: **Garis Sederhana** (yang dipakai di atas), **UnicodeMath**, dan **LaTeX**. Ini adalah dua cara menulis matematika sebagai teks biasa yang sudah dikenal banyak orang. Saat Anda memilih format lain, apa yang Anda ketik ditulis ulang dalam format tersebut.

- **UnicodeMath:** `a/b` adalah pecahan, `x^2_i` memiliki pangkat dan subskrip, `√(x)` adalah akar, dan `∑_(i=1)^n i` adalah jumlah. Tulis kata di antara tanda kutip, misalnya `"jika x"`, dan kata tersebut tetap berupa teks biasa, bukan huruf matematika. `(a|b)` membuat kurung dengan dua bagian, dan `\matrix(a&b@c&d)` membuat matriks (`&` di antara sel, `@` di antara baris).
- **LaTeX:** `\frac{a}{b}` adalah pecahan, `\sqrt[3]{x}` adalah akar pangkat tiga, `\sum_{i=1}^n i` adalah jumlah, `\text{jika x}` adalah teks biasa, dan `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` adalah matriks.

Dua tombol **Profesional** dan **Linear** mengubah tampilan persamaan di dalam kotak. Profesional menggambarnya tersusun, seperti tampilan nantinya. Linear hanya menampilkan baris yang Anda ketik. Jika sebuah persamaan tidak dapat ditulis dalam format yang Anda pilih, kotak akan memberi tahu Anda dan tetap menyimpan apa yang Anda ketik.

## Tab Persamaan

Pilih sebuah persamaan, dan tab bernama **Persamaan** muncul di akhir pita. Tab ini hilang saat Anda memilih hal lain.

- **Alat:** **Edit Persamaan** membuka kotak. **Profesional** dan **Linear** memilih cara kotak dibuka, tersusun atau hanya sebagai baris. **Teks Normal** membuka kotak dengan sepasang tanda kutip, untuk kata yang bukan matematika.
- **Konversi:** pilih format tempat kotak dibuka: **Garis Sederhana**, **UnicodeMath**, atau **LaTeX**. Saat Anda mengedit persamaan berikutnya, persamaan ditulis dalam format yang Anda pilih. Persamaan yang tidak dapat ditulis oleh format garis sederhana, misalnya jumlah dengan batas atas dan bawahnya, dibuka dalam UnicodeMath.
- **Simbol:** huruf Yunani, operator, relasi, panah, serta himpunan dan logika. Pilih simbol, dan kotak dibuka dengan simbol tersebut sudah ditambahkan ke baris.
- **Struktur:** pecahan, skrip, radikal, integral, operator besar, kurung, fungsi, aksen, batas, operator, dan matriks. Klik tombol untuk bentuk biasanya, atau panah di sampingnya untuk bentuk lain, misalnya turunan, akar dengan pangkat, atau matriks identitas.

Sebuah struktur disisipkan di akhir baris, dengan huruf contoh di dalamnya, dan bagian pertama dipilih sehingga Anda dapat langsung mengetik di atasnya. Kotak memiliki struktur yang sama dalam deretan tombol kecil di atas baris. Di sana, struktur disisipkan di posisi kursor, dan jika sebagian baris dipilih, bagian itu masuk ke posisi pertama struktur.

## Mengubah, memindahkan, dan mengatur ukuran

Klik dua kali persamaan, atau pilih persamaan lalu klik **Sisipkan > Simbol > Persamaan**, untuk membuka kotak dengan barisnya. Ubah isinya lalu klik **OK**. Perubahan ini dapat dibatalkan dalam satu langkah. Seret persamaan untuk memindahkannya, dan seret pegangan untuk mengubah ukurannya; persamaan tetap mempertahankan proporsinya di dalam kotak.

## Menyimpan

- **.xlsx** menyimpan persamaan sebagai persamaan, dalam bentuk standar untuk matematika, dengan gambar di sampingnya untuk program yang tidak dapat membaca matematika. Saat berkas dibuka kembali, persamaan tampil. Persamaan yang dibuat di program lain ditampilkan dengan cara yang sama; jika matematikanya tidak dapat dibaca, Anda melihat gambarnya.
- **PDF dan pencetakan** menggambar persamaan sebagai bentuk, sehingga tetap tajam pada ukuran berapa pun.
- Salinan **.ods** tidak memiliki persamaan. JeffCrunch menyimpan setiap persamaan sebagai gambar di tempat yang sama, dan menyebut nama persamaan saat memberi tahu Anda apa yang tidak ikut disalin.
