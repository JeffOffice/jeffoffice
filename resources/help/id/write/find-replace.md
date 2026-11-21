# Temukan dan ganti kata
<!-- keywords: temukan lanjutan, terdengar seperti, soundex, bentuk kata, semua bentuk kata, abaikan tanda baca, abaikan spasi, khusus, tanda paragraf, tab, temukan berdasarkan format, format, tebal, font, gaya, karakter pengganti, wildcard, pola, ekspresi reguler, grup, jumlah kecocokan, sorot semua kecocokan, ganti, ganti semua, cari, carikan, ubah kata, cocokkan huruf besar/kecil, kata utuh, temukan berikutnya, ctrl+f, ctrl+h, pengeditan, panel, teks tersembunyi, urungkan -->

Gunakan Temukan untuk melompat ke kata-kata dalam dokumen Anda. Gunakan Ganti untuk mengubahnya. Keduanya bekerja dalam satu panel di sisi kanan jendela.

## Menemukan kata

1. Tekan **Ctrl+F**, atau klik **Beranda > Pengeditan > Temukan**. Panel **Temukan dan Ganti** terbuka, dengan kursor berada di **Temukan apa**.
2. Ketik kata yang ingin Anda cari. Lalu tekan **Enter**, atau klik **Temukan Berikutnya**.

JeffWrite mulai dari posisi kursor Anda. Program memilih tempat berikutnya yang berisi kata Anda, lalu menggulir halaman agar Anda dapat melihatnya. Tempat itu disebut **kecocokan**. Klik **Temukan Berikutnya** lagi untuk ke kecocokan berikutnya. Setelah kecocokan terakhir, program kembali ke awal dokumen.

Saat Anda mengetik, panel menghitung kecocokan, misalnya "9 kecocokan", dan setiap kecocokan diberi pita kuning pada halaman. Pita itu juga tampil di sel tabel, header, footer, dan catatan kaki. Setelah **Temukan Berikutnya**, panel menyatakan kecocokan yang sedang Anda tuju, misalnya "Kecocokan 2 dari 4". Jumlah dan pita berubah saat Anda mengklik **Cocokkan huruf besar/kecil** atau **Temukan hanya kata utuh**, atau saat Anda mengubah teks. Keduanya hilang saat panel ditutup.

## Menemukan dengan karakter pengganti

Centang **Gunakan karakter pengganti** untuk mencari pola, bukan sekadar kata. **Karakter pengganti** adalah tanda yang mewakili huruf lain. Berikut tandanya:

- **?** mewakili satu huruf apa pun. **c?t** menemukan "cat" dan "cot".
- **\*** mewakili deretan huruf apa pun. **c\*t** menemukan "cat" dan "cart".
- **<** adalah awal kata, dan **>** adalah akhir kata. **<the>** menemukan "the" tetapi tidak "other".
- **[abc]** adalah salah satu huruf tersebut, **[a-c]** adalah huruf apa pun dari a sampai c, dan **[!abc]** adalah huruf apa pun selain itu.
- **{2}** mengulang huruf di depannya dua kali. **{1,3}** mengulanginya satu sampai tiga kali.
- **@** berarti satu atau lebih huruf di depannya.
- **\\** di depan tanda akan menemukan tanda itu sendiri. **\\?** menemukan tanda tanya.
- **^p** adalah tanda paragraf, **^t** adalah tab, **^#** adalah angka, **^$** adalah huruf, dan **^w** adalah spasi kosong.

Letakkan sebagian pola dalam **( )** untuk membuat **grup**. Pada **Ganti dengan**, **\1** adalah grup pertama, **\2** adalah grup kedua, dan seterusnya sampai **\9**. **^&** adalah seluruh teks yang ditemukan. Misalnya, cari **(<[A-Z]@>), (<[A-Z]@>)** dan ganti dengan **\2 \1** untuk mengubah "Smith, John" menjadi "John Smith". **Temukan hanya kata utuh** dinonaktifkan selama karakter pengganti aktif, karena **<** dan **>** sudah melakukan tugas itu.

Jika pola memiliki kesalahan, panel akan menjelaskannya.

Jika kata Anda tidak ada dalam dokumen, panel akan memberi tahu, dan tidak ada yang berpindah.

Anda tetap dapat bekerja dalam dokumen saat panel terbuka. Klik halaman untuk mengetik. Klik **X** di bagian atas panel untuk menutupnya.

## Temukan lanjutan

Pilihan ini ada di panel, di bawah **Gunakan karakter pengganti**. Setiap pilihan mengubah cara JeffWrite membaca kata dalam **Temukan apa**.

- **Terdengar seperti** menemukan kata yang dilafalkan seperti kata Anda. Program memberi setiap kata kode dari bunyinya, lalu mencari kata dengan kode yang sama. Ketik **Smith**, dan program juga menemukan "Smyth".
- **Temukan semua bentuk kata** menemukan setiap bentuk kata. Ketik **run**, dan program menemukan "runs," "running," dan "ran." Pilihan ini bekerja pada kata utuh. Jika Anda mengetik dua kata, misalnya **big dog**, program menemukannya berdampingan, dalam bentuk apa pun.
- **Abaikan karakter tanda baca** melewati tanda seperti tanda hubung dan titik di antara huruf. Ketik **email**, dan program juga menemukan "e-mail".
- **Abaikan karakter spasi kosong** melewati spasi dan tab di antara huruf. Ketik **e mail**, dan program menemukan "email".

Hanya satu dari **Terdengar seperti**, **Temukan semua bentuk kata**, dan **Gunakan karakter pengganti** yang dapat aktif pada satu waktu. **Terdengar seperti** dan **Temukan semua bentuk kata** selalu mencari kata utuh, sehingga **Temukan hanya kata utuh** dan **Cocokkan huruf besar/kecil** dinonaktifkan.

### Khusus

Klik **Khusus** untuk memasukkan kode ke dalam **Temukan apa**: **Tanda Paragraf** (^p), **Karakter Tab** (^t), **Angka Apa Pun** (^#), **Huruf Apa Pun** (^$), **Spasi Kosong** (^w), **Pemisah Bagian** (^b), **Bidang** (^d), **Tanda Catatan Kaki** (^f), **Tanda Catatan Akhir** (^e), atau **Karakter Caret** (^^). Kode ini bekerja tanpa **Gunakan karakter pengganti**. **Pemisah bagian** adalah tanda yang mengakhiri sebuah bagian. **Bidang** adalah potongan teks yang dihitung oleh program, seperti nomor halaman atau tanggal, dan **^d** menemukan tempat setiap bidang dimulai. Tanda catatan adalah angka kecil dalam teks yang menunjuk ke catatan kaki atau catatan akhir. Keempat tanda ini dapat ditemukan, tetapi **Ganti** dan **Ganti Semua** tidak mengubahnya, karena perubahan akan menghilangkan apa yang diwakilinya. Tanda lain dalam kotak, seperti **?** dan **\***, hanya berarti karakter tersebut.

### Format

Klik **Format** untuk hanya menemukan teks dengan tampilan tertentu: **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Disorot**, sebuah **Font**, atau **Gaya** paragraf. Panel mencantumkan tampilan yang Anda pilih di bawah tombol. Pilih **Tanpa Pemformatan** untuk menghapusnya.

Anda dapat mengosongkan **Temukan apa**. Maka setiap bagian teks dengan tampilan itu menjadi kecocokan. Jika ada kata dalam **Temukan apa**, hanya kata dengan tampilan tersebut yang ditemukan. **Ganti Semua** hanya mengubah kata-kata tersebut.

## Mempersempit pencarian

- **Cocokkan huruf besar/kecil** hanya menemukan kata dengan huruf kapital yang sama. Saat aktif, "Rencana" tidak menemukan "rencana".
- **Temukan hanya kata utuh** melewati kata yang lebih panjang yang memuat kata Anda. Saat aktif, "rencana" tidak menemukan "perencanaan".
- **Cari** menentukan arah **Temukan Berikutnya** dari kursor. **Ke bawah** menuju akhir dokumen, dan **Ke atas** menuju awal. Pencarian berhenti di sana dan memberi tahu Anda. **Semua** melanjutkan ke ujung lainnya dan merupakan pilihan yang umum. **Ganti Semua** selalu mengubah seluruh dokumen.

## Mengganti kata

1. Tekan **Ctrl+H**, atau klik **Beranda > Pengeditan > Ganti**. Kursor berpindah ke **Ganti dengan**.
2. Ketik kata baru di **Ganti dengan**. Untuk menghapus kata yang ditemukan, biarkan kosong.
3. Ketik kata lama di **Temukan apa**.
4. Klik **Ganti** untuk melihat kecocokan pertama. Klik **Ganti** lagi untuk mengubahnya. JeffWrite lalu berpindah ke kecocokan berikutnya. Setiap perubahan dihitung sebagai satu langkah, sehingga **Urungkan** (Ctrl+Z) membatalkannya satu per satu.
5. Klik **Ganti Semua** untuk mengubah setiap kecocokan sekaligus. Panel memberi tahu berapa banyak yang diubah. **Ganti Semua** juga mengubah kata dalam kotak teks dan bentuk, dan dihitung sebagai satu langkah, sehingga satu **Urungkan** mengembalikan semua kata. Saat **Lacak Perubahan** aktif, setiap penggantian dicatat sebagai perubahan yang dilacak, baik dalam teks maupun dalam kotak. Kotak yang terlalu besar untuk diubah dihitung dalam pesan. **Ganti Semua** tidak menjangkau kotak teks di header dan footer, bentuk yang dikelompokkan, atau kotak yang saling tertaut. **Temukan Berikutnya** belum mencari di dalam kotak teks.

Kata baru mengikuti huruf pertama kata yang digantinya. Jika Anda mengganti kata yang tebal, kata baru juga akan tebal.

Misalnya, untuk mengubah "color" menjadi "colour" di seluruh dokumen: ketik **color** di **Temukan apa**, ketik **colour** di **Ganti dengan**, lalu klik **Ganti Semua**.

## Yang diperiksa pencarian

- Pencarian memeriksa teks dokumen, termasuk teks dalam tabel. Penanda di tengah beberapa kata, yang dimiliki sebagian berkas, tidak menghalangi kata tersebut ditemukan. **Ganti** dan **Ganti Semua** mempertahankan penanda.
- **Temukan Berikutnya** memeriksa teks lebih dulu. Lalu dilanjutkan ke header, footer, catatan kaki, dan kembali ke awal teks. Saat kecocokan berada di salah satunya, JeffWrite membukanya untuk diedit dan memilih kecocokan itu. Panel memberi tahu lokasinya, misalnya "Ditemukan di header halaman 3." **Ganti** menukar kecocokan yang Anda tuju, baik di teks maupun di header, footer, atau catatan kaki. **Ganti Semua** mengubah setiap kecocokan di semuanya, dan satu **Urungkan** mengembalikan semuanya. Catatan akhir merupakan bagian dari teks, sehingga ikut dicari. Lihat [Header dan footer](headers-footers).
- Pencarian melewati teks yang diatur sebagai **Tersembunyi**, karena teks itu tidak ditampilkan. Lihat [Mengubah tampilan teks](font).

## Terkait

- [Mengubah tampilan teks](font)
- [Menggunakan JeffWrite dengan papan ketik](keyboard)
