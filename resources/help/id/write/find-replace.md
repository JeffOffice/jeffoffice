# Menemukan dan mengganti kata
<!-- keywords: pencarian lanjutan, terdengar seperti, soundex, bentuk kata, semua bentuk kata, abaikan tanda baca, abaikan spasi, khusus, tanda paragraf, tab, cari berdasarkan format, format, tebal, font, gaya, karakter pengganti, wildcard, pola, ekspresi reguler, grup, jumlah kecocokan, sorot semua kecocokan, ganti, ganti semua, cari, temukan, ubah kata, cocokkan huruf besar/kecil, kata utuh, temukan berikutnya, ctrl+f, ctrl+h, pengeditan, panel, teks tersembunyi, urungkan -->

Gunakan Temukan untuk melompat ke kata-kata dalam dokumen Anda. Gunakan Ganti untuk mengubahnya. Keduanya berfungsi dalam satu panel di sisi kanan jendela.

## Mencari kata

1. Tekan **Ctrl+F**, atau klik **Beranda > Pengeditan > Temukan**. Panel **Temukan dan Ganti** terbuka, dengan kursor berada di **Temukan apa**.
2. Ketik kata yang ingin Anda cari. Lalu tekan **Enter**, atau klik **Temukan Berikutnya**.

JeffWrite mulai dari posisi kursor Anda. JeffWrite memilih tempat berikutnya yang memuat kata Anda, dan menggulir halaman agar Anda dapat melihatnya. Tempat itu disebut **kecocokan**. Klik **Temukan Berikutnya** lagi untuk pergi ke kecocokan berikutnya. Setelah kecocokan terakhir, JeffWrite kembali ke awal dokumen.

Saat Anda mengetik, panel menghitung kecocokan, misalnya "9 kecocokan", dan setiap kecocokan mendapat pita kuning pada halaman. Pita tersebut juga tampil di sel tabel, header, footer, dan catatan kaki. Setelah **Temukan Berikutnya**, panel menyebutkan kecocokan yang sedang Anda tuju, misalnya "Kecocokan 2 dari 4". Jumlah dan pita berubah saat Anda mengklik **Cocokkan huruf besar/kecil** atau **Temukan hanya kata utuh**, atau saat Anda mengubah teks. Keduanya hilang saat Anda menutup panel.

## Mencari dengan karakter pengganti

Centang **Gunakan wildcard** untuk mencari pola, bukan hanya kata. **Wildcard** adalah tanda yang mewakili huruf lain. Berikut tanda-tandanya:

- **?** mewakili satu huruf apa pun. **c?t** menemukan "cat" dan "cot".
- **\*** mewakili deretan huruf apa pun. **c\*t** menemukan "cat" dan "cart".
- **<** adalah awal kata, dan **>** adalah akhir kata. **<the>** menemukan "the" tetapi tidak "other".
- **[abc]** adalah salah satu huruf tersebut, **[a-c]** adalah huruf apa pun dari a sampai c, dan **[!abc]** adalah huruf apa pun selain huruf tersebut.
- **{2}** mengulang huruf di depannya dua kali. **{1,3}** mengulangnya satu sampai tiga kali.
- **@** berarti satu atau lebih huruf di depannya.
- **\\** di depan tanda menemukan tanda itu sendiri. **\\?** menemukan tanda tanya.
- **^p** adalah tanda paragraf, **^t** adalah tab, **^#** adalah angka, **^$** adalah huruf, dan **^w** adalah spasi kosong.

Letakkan sebagian pola di dalam **( )** untuk membuat **grup**. Di **Ganti dengan**, **\1** adalah grup pertama, **\2** adalah grup kedua, dan seterusnya sampai **\9**. **^&** adalah seluruh teks yang ditemukan. Misalnya, cari **(<[A-Z]@>), (<[A-Z]@>)** dan ganti dengan **\2 \1** untuk mengubah "Smith, John" menjadi "John Smith". **Temukan hanya kata utuh** diredupkan selama wildcard aktif, karena **<** dan **>** sudah melakukan tugas itu.

Jika pola mengandung kesalahan, panel akan menjelaskan kesalahannya.

Jika kata Anda tidak ada dalam dokumen, panel akan memberi tahu, dan tidak ada yang berpindah.

Anda tetap dapat bekerja di dokumen selama panel terbuka. Klik halaman untuk mengetik. Klik **X** di bagian atas panel untuk menutupnya.

## Temukan Lanjutan

Pilihan-pilihan ini ada di panel, di bawah **Gunakan wildcard**. Masing-masing mengubah cara JeffWrite membaca kata dalam **Temukan apa**.

- **Terdengar seperti** menemukan kata yang diucapkan dengan bunyi yang sama dengan kata Anda. Setiap kata diberi kode berdasarkan bunyinya, lalu kata dengan kode yang sama ditemukan. Ketik **Smith**, dan "Smyth" juga ditemukan.
- **Temukan semua bentuk kata** menemukan setiap bentuk dari kata tersebut. Ketik **run**, dan "runs," "running," dan "ran" ditemukan. Fitur ini bekerja pada kata utuh. Jika Anda mengetik dua kata, seperti **big dog**, keduanya ditemukan berdampingan, dalam bentuk apa pun.
- **Abaikan karakter tanda baca** melewati tanda seperti tanda hubung dan titik di antara huruf. Ketik **email**, dan "e-mail" juga ditemukan.
- **Abaikan karakter spasi kosong** melewati spasi dan tab di antara huruf. Ketik **e mail**, dan "email" ditemukan.

Hanya satu dari **Terdengar seperti**, **Temukan semua bentuk kata**, dan **Gunakan wildcard** yang dapat aktif pada satu waktu. **Terdengar seperti** dan **Temukan semua bentuk kata** selalu mencari kata utuh, sehingga **Temukan hanya kata utuh** dan **Cocokkan huruf besar/kecil** diredupkan.

### Khusus

Klik **Khusus** untuk memasukkan kode ke dalam **Temukan apa**: **Tanda Paragraf** (^p), **Karakter Tab** (^t), **Digit Apa Pun** (^#), **Huruf Apa Pun** (^$), **Spasi Kosong** (^w), **Pemisah Bagian** (^b), **Bidang** (^d), **Tanda Catatan Kaki** (^f), **Tanda Catatan Akhir** (^e), atau **Karakter Caret** (^^). Kode-kode ini bekerja tanpa **Gunakan wildcard**. **Pemisah bagian** adalah tanda yang mengakhiri sebuah bagian. **Bidang** adalah potongan teks yang dihitung oleh program, seperti nomor halaman atau tanggal, dan **^d** menemukan tempat setiap bidang dimulai. Tanda catatan adalah angka kecil dalam teks yang menunjuk ke catatan kaki atau catatan akhir. Keempat tanda ini dapat ditemukan, tetapi **Ganti** dan **Ganti Semua** tidak mengubahnya, karena mengubah salah satunya berarti kehilangan makna yang diwakilinya. Tanda lain dalam kotak, seperti **?** dan **\***, kemudian hanya berarti karakter itu sendiri.

### Format

Klik **Format** untuk hanya menemukan teks dengan tampilan tertentu: **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Disorot**, **Font**, atau **Gaya** paragraf. Panel mencantumkan tampilan yang Anda pilih di bawah tombol. Pilih **Tanpa Pemformatan** untuk menghapusnya.

Anda dapat membiarkan **Temukan apa** kosong. Maka setiap rangkaian teks dengan tampilan tersebut menjadi kecocokan. Jika ada kata dalam **Temukan apa**, hanya kata dengan tampilan tersebut yang ditemukan. **Ganti Semua** hanya mengubah kata-kata tersebut.

## Mempersempit pencarian

- **Cocokkan huruf besar/kecil** hanya menemukan kata dengan huruf kapital yang sama. Jika opsi ini aktif, "Rencana" tidak menemukan "rencana."
- **Temukan hanya kata utuh** melewati kata yang lebih panjang dan memuat kata Anda. Jika opsi ini aktif, "rencana" tidak menemukan "rencanakan."

## Mengganti kata

1. Tekan **Ctrl+H**, atau klik **Beranda > Pengeditan > Ganti**. Kursor berpindah ke **Ganti dengan**.
2. Ketik kata baru di **Ganti dengan**. Untuk menghapus kata yang ditemukan, biarkan kosong.
3. Ketik kata lama di **Temukan apa**.
4. Klik **Ganti** untuk melihat kecocokan pertama. Klik **Ganti** lagi untuk mengubahnya. JeffWrite kemudian berpindah ke kecocokan berikutnya. Setiap perubahan adalah satu langkah, sehingga **Urungkan** (Ctrl+Z) membatalkannya satu per satu.
5. Klik **Ganti Semua** untuk mengubah setiap kecocokan sekaligus. Panel memberi tahu berapa banyak yang berubah. **Ganti Semua** adalah satu langkah, sehingga satu kali **Urungkan** mengembalikan semua kata.

Kata baru mengikuti huruf pertama kata yang digantinya. Jika Anda mengganti kata yang dicetak tebal, kata baru tersebut juga dicetak tebal.

Sebagai contoh, untuk mengubah "color" menjadi "colour" di seluruh dokumen: ketik **color** di **Temukan apa**, ketik **colour** di **Ganti dengan**, lalu klik **Ganti Semua**.

## Yang diperiksa pencarian

- Pencarian memeriksa teks dokumen, termasuk teks dalam tabel. Penanda di tengah beberapa kata, yang dimiliki sebagian berkas, tidak menghalangi kata tersebut untuk ditemukan. **Ganti** dan **Ganti Semua** mempertahankan penanda tersebut.
- **Temukan Berikutnya** memeriksa teks terlebih dahulu. Setelah itu, pencarian dilanjutkan ke header, lalu footer, lalu catatan kaki, dan kembali ke awal teks. Jika kecocokan berada di salah satunya, JeffWrite membukanya untuk diedit dan memilih kecocokan tersebut. Panel memberi tahu lokasinya, misalnya "Ditemukan di header halaman 3." **Ganti** menukar kecocokan yang sedang Anda tuju, baik di teks maupun di header, footer, atau catatan kaki. **Ganti Semua** mengubah setiap kecocokan di semua bagian tersebut, dan satu kali **Urungkan** mengembalikan semuanya. Catatan akhir merupakan bagian dari teks, sehingga ikut dicari bersama teks. Lihat [Header dan footer](headers-footers).
- Pencarian melewati teks yang diatur ke **Tersembunyi**, karena teks tersebut tidak ditampilkan. Lihat [Ubah tampilan teks](font).

## Terkait

- [Ubah tampilan teks](font)
- [Gunakan JeffWrite dengan papan ketik](keyboard)
