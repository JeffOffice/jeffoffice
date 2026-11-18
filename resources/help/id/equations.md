# Menambahkan persamaan
<!-- keywords: tab persamaan, struktur, simbol, konversi, persamaan, matematika, rumus, pecahan, akar kuadrat, superskrip, subskrip, pangkat, jumlah, integral, yunani, simbol, sisipkan persamaan, alt+=, aksen, matriks, kurung, persamaan bawaan, galeri, ubah persamaan, edit persamaan, vektor, pdf, jpub, berkas pub -->

Sebuah **persamaan** adalah satu baris matematika, seperti pecahan atau akar kuadrat. Di JeffPub, persamaan adalah objek pada halaman, seperti gambar: Anda dapat memindahkannya, mengubah ukurannya, memutarnya, dan memberinya warna.

## Menempatkan persamaan pada halaman

1. Klik **Sisipkan > Teks > Persamaan**, atau tekan **Alt+=**.
2. Ketik persamaan dalam satu baris. Di bawah kotak, Anda melihat bagaimana tampilannya nanti. Klik **OK**.

Persamaan diletakkan di tengah halaman dalam keadaan terpilih. Untuk memulai dari persamaan yang sudah dikenal, klik panah di sebelah **Persamaan**, lalu pilih persamaan bawaan, seperti rumus kuadrat atau luas lingkaran.

## Cara mengetiknya

- **Pecahan:** ketik garis miring. `a/b` berarti a per b. `(x+1)/(x-1)` menganggap setiap kelompok dalam kurung sebagai satu bagian.
- **Pangkat dan subskrip:** `x^2` berarti x kuadrat, dan `x_i` berarti x dengan i kecil di bawahnya. Keduanya dapat dipakai bersamaan, seperti `x_i^2`.
- **Akar:** ketik `sqrt(x+1)` untuk akar kuadrat, atau `sqrt[3]{x}` untuk akar pangkat tiga.
- **Jumlah, integral, dan perkalian:** ketik `\sum_(i=1)^n i`, `\int_0^1 x`, atau `\prod_(k=1)^n k`. Bagian setelah garis bawah diletakkan di bawah, dan bagian setelah tanda pangkat diletakkan di atas.
- **Simbol:** ketik garis miring terbalik dan sebuah nama. `\alpha` menjadi α, `\pi` menjadi π, `\infty` menjadi ∞, `\leq` menjadi ≤, dan `\times` menjadi ×. Sekitar 70 nama dapat digunakan.
- **Aksen:** `\hat{x}`, `\vec{v}`, dan `\bar{x}` menaruh tanda di atas huruf.
- **Kurung yang membesar:** ketik `\left( ... \right)`, dan kurung akan membesar sesuai tinggi isi di antaranya.
- **Matriks:** **matriks** adalah kisi angka. Ketik `\pmatrix{a & b \\ c & d}`. Letakkan `&` di antara sel dalam satu baris, dan dua garis miring terbalik di antara baris.

## Mengetik dalam UnicodeMath atau LaTeX

Di sebelah kotak terdapat daftar format pengetikan: **Baris Sederhana** (yang di atas), **UnicodeMath**, dan **LaTeX**. Keduanya adalah dua cara menulis matematika sebagai teks biasa yang sudah dikenal banyak orang. Ketika Anda memilih format lain, apa yang Anda ketik ditulis ulang dalam format tersebut.

- **UnicodeMath:** `a/b` adalah pecahan, `x^2_i` memiliki pangkat dan subskrip, `√(x)` adalah akar, dan `∑_(i=1)^n i` adalah jumlah. Letakkan kata dalam tanda kutip, seperti `"jika x"`, maka kata tersebut tetap berupa teks biasa, bukan huruf matematika. `(a|b)` membuat kurung dengan dua bagian, dan `\matrix(a&b@c&d)` membuat matriks (`&` di antara sel, `@` di antara baris).
- **LaTeX:** `\frac{a}{b}` adalah pecahan, `\sqrt[3]{x}` adalah akar pangkat tiga, `\sum_{i=1}^n i` adalah jumlah, `\text{jika x}` adalah teks biasa, dan `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` adalah matriks.

Dua tombol **Profesional** dan **Linier** mengubah cara kotak menampilkan persamaan Anda. Profesional menggambarnya secara bertingkat, seperti tampilannya nanti. Linier hanya menampilkan baris yang Anda ketik. Jika sebuah persamaan tidak dapat ditulis dalam format yang Anda pilih, kotak akan memberi tahu Anda dan tetap menyimpan apa yang Anda ketik.

## Mengubah persamaan

Pilih persamaan lalu tekan Enter, atau klik dua kali, atau klik **Sisipkan > Teks > Persamaan**. Kotak akan terbuka dengan persamaan yang ditulis dalam satu baris. Ubah isinya lalu klik **OK**. Perubahan ini dapat dibatalkan dalam satu langkah.

Persamaan yang berasal dari berkas akan terbuka dalam UnicodeMath, atau dalam LaTeX jika UnicodeMath tidak dapat menuliskannya. Jika keduanya tidak dapat, kotak akan dimulai dengan teks persamaan tersebut. Klik **OK** tanpa perubahan, atau **Batal**, untuk membiarkan persamaan tetap seperti semula, atau ketik persamaan baru untuk menggantinya.

## Tab Persamaan

Pilih sebuah persamaan, dan tab bernama **Persamaan** akan muncul di ujung pita. Tab ini hilang ketika Anda memilih hal lain.

- **Alat:** **Edit Persamaan** membuka kotak. **Profesional** dan **Linier** mengubah cara persamaan yang dipilih ditampilkan pada halaman: Profesional menggambarnya secara bertingkat, dan Linier menampilkan baris tempat persamaan itu diketik. Pilihan ini disimpan bersama persamaan dalam berkas `.jpub`, sehingga tetap sama ketika berkas dibuka kembali. Berkas `.pub` akan menerima gambar persamaan sebagaimana tampilannya di halaman. **Teks Normal** membuka kotak dengan sepasang tanda kutip ditambahkan, untuk kata-kata yang bukan matematika.
- **Konversi:** pilih format tempat kotak dibuka: **Baris Sederhana**, **UnicodeMath**, atau **LaTeX**. Saat Anda mengedit persamaan berikutnya, persamaan akan ditulis dalam format yang Anda pilih. Persamaan yang tidak dapat ditulis oleh format baris sederhana, seperti jumlah dengan batas atas dan bawahnya, akan dibuka dalam UnicodeMath sebagai gantinya.
- **Simbol:** huruf Yunani, operator, relasi, panah, serta himpunan dan logika. Pilih sebuah simbol, dan kotak akan terbuka dengan simbol tersebut sudah ditambahkan pada baris.
- **Struktur:** pecahan, skrip, radikal, integral, operator besar, kurung, fungsi, aksen, batas, operator, dan matriks. Klik sebuah tombol untuk bentuk biasanya, atau panah di sebelahnya untuk bentuk lain, seperti turunan, akar dengan pangkat, atau matriks identitas.

Struktur dimasukkan di ujung baris, dengan huruf contoh di dalamnya, dan bagian pertama dipilih sehingga Anda dapat langsung mengetik di atasnya. Kotak memiliki struktur yang sama dalam satu baris tombol kecil di atas baris. Di sana, struktur dimasukkan pada posisi kursor, dan jika sebagian baris dipilih, bagian itu masuk ke posisi pertama struktur.

## Memindahkan, mengubah ukuran, dan memberi warna

Seret persamaan untuk memindahkannya. Seret sudut untuk memperbesar atau memperkecil; persamaan tetap mempertahankan proporsinya. Gunakan **Isian Bentuk** untuk mengubah warna huruf.

## Menyimpan

- **.jpub** menyimpan persamaan, sehingga Anda dapat mengeditnya lagi.
- **PDF dan pencetakan** menggambar persamaan sebagai bentuk, sehingga tetap tajam pada ukuran berapa pun.
- **.pub** tidak memiliki persamaan. JeffPub menyimpan setiap persamaan sebagai gambar, di tempat dan ukuran yang sama, dan memberi tahu Anda saat menyimpan.
- **EPUB** dan **HTML** menyimpan persamaan sebagai baris yang Anda ketik.
