# Temukan dan ganti kata
<!-- keywords: temukan lanjutan, terdengar seperti, soundex, bentuk kata, semua bentuk kata, abaikan tanda baca, abaikan spasi, khusus, tanda paragraf, tab, temukan berdasarkan format, format, tebal, font, gaya, wildcard, pola, ekspresi reguler, grup, jumlah kecocokan, sorot semua kecocokan, ganti, ganti semua, cari, ubah kata, cocokkan huruf besar/kecil, kata utuh, temukan berikutnya, ctrl+f, ctrl+h, pengeditan, panel, teks tersembunyi, urungkan -->

Gunakan Temukan untuk melompat ke kata-kata dalam dokumen Anda. Gunakan Ganti untuk mengubahnya. Keduanya bekerja dalam satu panel di sisi kanan jendela.

## Temukan kata

1. Tekan **Ctrl+F**, atau klik **Beranda > Pengeditan > Temukan**. Panel **Temukan dan Ganti** terbuka, dengan kursor berada di **Temukan apa**.
2. Ketikkan kata yang ingin Anda cari. Lalu tekan **Enter**, atau klik **Temukan Berikutnya**.

JeffWrite mulai dari posisi kursor Anda. Program memilih tempat berikutnya yang memuat kata Anda, lalu menggulir halaman agar Anda dapat melihatnya. Tempat itu disebut **kecocokan**. Klik **Temukan Berikutnya** lagi untuk pindah ke kecocokan berikutnya. Setelah kecocokan terakhir, program kembali ke awal dokumen.

Saat Anda mengetik, panel menghitung kecocokan, misalnya "9 kecocokan", dan setiap kecocokan diberi pita kuning pada halaman. Pita tersebut juga tampil di sel tabel, header, footer, dan catatan kaki. Setelah **Temukan Berikutnya**, panel menyebutkan kecocokan yang sedang Anda lihat, misalnya "Kecocokan 2 dari 4". Jumlah dan pita kuning berubah saat Anda mengklik **Cocokkan huruf besar/kecil** atau **Temukan hanya kata utuh**, atau saat Anda mengubah teks. Keduanya hilang saat panel ditutup.

## Temukan dengan wildcard

Centang **Gunakan wildcard** untuk mencari pola, tidak hanya kata. **Wildcard** adalah tanda yang mewakili huruf lain. Berikut tanda-tandanya:

- **?** mewakili satu huruf apa pun. **c?t** menemukan "cat" dan "cot".
- **\*** mewakili rangkaian huruf apa pun. **c\*t** menemukan "cat" dan "cart".
- **<** adalah awal kata, dan **>** adalah akhir kata. **<the>** menemukan "the" tetapi bukan "other".
- **[abc]** adalah salah satu huruf tersebut, **[a-c]** adalah huruf apa pun dari a sampai c, dan **[!abc]** adalah huruf apa pun selain huruf tersebut.
- **{2}** mengulang huruf di depannya dua kali. **{1,3}** mengulangnya satu sampai tiga kali.
- **@** berarti satu atau lebih huruf di depannya.
- **\\** di depan tanda akan mencari tanda itu sendiri. **\\?** menemukan tanda tanya.
- **^p** adalah tanda paragraf, **^t** adalah tab, **^#** adalah angka, **^$** adalah huruf, dan **^w** adalah spasi kosong.

Letakkan sebagian pola di dalam **( )** untuk membuat **grup**. Di **Ganti dengan**, **\1** adalah grup pertama, **\2** adalah grup kedua, dan seterusnya sampai **\9**. **^&** adalah seluruh teks yang ditemukan. Misalnya, temukan **(<[A-Z]@>), (<[A-Z]@>)** dan ganti dengan **\2 \1** untuk mengubah "Smith, John" menjadi "John Smith". **Temukan hanya kata utuh** dinonaktifkan selama wildcard aktif, karena **<** dan **>** sudah melakukan tugas itu.

Jika pola memiliki kesalahan, panel akan menyebutkan kesalahannya.

Jika kata Anda tidak ada dalam dokumen, panel akan memberi tahu, dan tidak ada yang berpindah.

Anda dapat terus bekerja di dokumen selagi panel terbuka. Klik halaman untuk mengetik. Klik **X** di bagian atas panel untuk menutupnya.

## Temukan dengan ekspresi reguler

Centang **Gunakan ekspresi reguler** untuk pola yang lebih rumit daripada wildcard. **Ekspresi reguler** adalah pola singkat yang terdiri dari huruf dan tanda. Pilihan ini diaktifkan sebagai pengganti **Gunakan wildcard**: hanya salah satu dari keduanya yang dapat aktif. Berikut tanda-tanda yang umum digunakan:

- **.** adalah satu karakter apa pun, **\d** adalah angka, **\w** adalah huruf, angka, atau garis bawah, dan **\s** adalah spasi atau tab. **[a-c]** adalah huruf apa pun dari a sampai c, dan **[^abc]** adalah karakter apa pun selain huruf tersebut.
- **^** adalah awal paragraf, **$** adalah akhir paragraf, dan **\b** adalah batas kata. **\bcat\b** menemukan "cat" tetapi bukan "category."
- **\*** berarti nol atau lebih dari yang ada di depannya, **+** berarti satu atau lebih, dan **?** berarti nol atau satu. **{2}** berarti tepat dua kali, dan **{2,5}** berarti dua sampai lima kali; jumlah pengulangan dapat mencapai 200. Tambahkan **?** setelah pengulangan, seperti **.+?**, untuk menemukan rangkaian terpendek yang cocok.
- **|** berarti "atau": **cat|dog** menemukan salah satu dari kedua kata.
- **( )** membuat **grup**. Gunakan **(?: )** untuk grup yang tidak Anda perlukan di **Ganti dengan**.
- **\\** di depan tanda akan mencari tanda itu sendiri: **\.** menemukan titik. **\t** adalah tab.

**Cocokkan huruf besar/kecil** bekerja seperti pada kata. Huruf dengan aksen dan huruf dari abjad lain dihitung sebagai huruf.

Pola dicari satu paragraf demi satu paragraf, sehingga **^** dan **$** berarti awal dan akhir setiap paragraf. Untuk mencari melewati akhir paragraf, masukkan **\n** (tanda paragraf) dalam pola: **home\.\nLee** menemukan "home." di akhir satu paragraf dan "Lee" di awal paragraf berikutnya.

Di **Ganti dengan**, **$1** atau **\1** adalah grup pertama, **$2** atau **\2** adalah grup kedua, dan seterusnya sampai sembilan grup. **$0** atau **\0** adalah seluruh teks yang ditemukan. **$$** adalah tanda dolar, **\\** adalah garis miring terbalik, dan **\t** adalah tab. Misalnya, temukan **(\w+), (\w+)** dan ganti dengan **$2 $1** untuk mengubah "Smith, John" menjadi "John Smith."

**Ganti Semua** mengubah setiap kecocokan dalam teks, header, footer, catatan, dan kotak teks sebagai satu langkah, sehingga satu **Urungkan** membatalkan semuanya. Jika Lacak Perubahan aktif, setiap perubahan dilacak, seperti pada kata. Teks tersembunyi tidak diubah.

Jika pola memiliki kesalahan, panel akan menyebutkan di dekat karakter mana kesalahan itu. Sebuah pola dapat tidak mencocokkan karakter apa pun, seperti **^** (awal paragraf) atau **$** (akhir paragraf): **Ganti dengan** kemudian memasukkan teksnya di posisi tersebut, sehingga **^** dengan **> ** menambahkan tanda itu di depan setiap paragraf. Pola yang akan memakan waktu terlalu lama pada teks Anda, seperti **(a+)+$** pada satu baris panjang berisi huruf a, akan dihentikan oleh JeffWrite, yang akan memberi tahu Anda; tidak ada yang diubah. Pencarian yang panjang menampilkan kotak **Mencari** dengan tombol **Batal**; membatalkannya membiarkan dokumen Anda seperti semula, dan Anda tidak dapat mengetik di dokumen sampai pencarian selesai. Pola yang melihat ke depan atau ke belakang tanpa memakai apa yang diperiksanya, seperti **x(?=\d)**, dan pola dalam mode diperluas, seperti **(?x)**, tidak didukung; JeffWrite akan memberi tahu Anda. Paragraf yang sangat panjang dicari sebagian demi sebagian, dan setiap kecocokan sama dengan yang akan ditemukan jika seluruh paragraf dicari. Pola yang cocok dengan rangkaian sangat panjang dalam paragraf yang sangat panjang (puluhan ribu karakter) dapat dihentikan dengan pesan "membutuhkan terlalu banyak proses", dan satu kecocokan lebih dari 200.000 karakter tidak diganti: JeffWrite berhenti, memberi tahu Anda, dan tidak mengubah apa pun.

## Temukan lanjutan

Pilihan berikut ada di panel di bawah **Gunakan ekspresi reguler**. Setiap pilihan mengubah cara JeffWrite membaca kata dalam **Temukan apa**.

- **Terdengar seperti** menemukan kata yang diucapkan dengan cara yang sama dengan kata Anda. Program memberi setiap kata kode dari bunyinya, lalu menemukan kata dengan kode yang sama. Ketik **Smith**, dan program juga menemukan "Smyth".
- **Temukan semua bentuk kata** menemukan setiap bentuk dari kata tersebut. Ketik **run**, dan program menemukan "runs," "running," dan "ran." Pilihan ini bekerja pada kata utuh. Jika Anda mengetik dua kata, seperti **big dog**, program menemukannya berdampingan, dalam bentuk apa pun.
- **Abaikan karakter tanda baca** melewati tanda seperti tanda hubung dan titik di antara huruf. Ketik **email**, dan program juga menemukan "e-mail".
- **Abaikan karakter spasi kosong** melewati spasi dan tab di antara huruf. Ketik **e mail**, dan program menemukan "email".

Hanya satu dari **Terdengar seperti**, **Temukan semua bentuk kata**, **Gunakan wildcard**, dan **Gunakan ekspresi reguler** yang dapat aktif pada satu waktu. **Terdengar seperti** dan **Temukan semua bentuk kata** selalu mencari kata utuh, sehingga **Temukan hanya kata utuh** dan **Cocokkan huruf besar/kecil** dinonaktifkan.

### Khusus

Klik **Khusus** untuk memasukkan kode ke dalam **Temukan apa**: **Tanda Paragraf** (^p), **Karakter Tab** (^t), **Angka Apa Pun** (^#), **Huruf Apa Pun** (^$), **Spasi Kosong** (^w), **Pemisah Bagian** (^b), **Bidang** (^d), **Tanda Catatan Kaki** (^f), **Tanda Catatan Akhir** (^e), atau **Karakter Kursor** (^^). Kode-kode ini bekerja tanpa **Gunakan wildcard**. Pemisah bagian adalah tanda yang mengakhiri sebuah bagian. Bidang adalah potongan teks yang dihitung oleh program, seperti nomor halaman atau tanggal, dan **^d** menemukan tempat setiap bidang dimulai. Tanda catatan adalah angka kecil dalam teks yang menunjuk ke catatan kaki atau catatan akhir. Keempat hal ini dapat ditemukan, tetapi **Ganti** dan **Ganti Semua** tidak mengubahnya, karena mengubah salah satunya akan menghilangkan apa yang diwakilinya. Tanda lain dalam kotak, seperti **?** dan **\***, kemudian hanya berarti karakter tersebut.

### Format

Klik **Format** untuk hanya menemukan teks dengan tampilan tertentu: **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Disorot**, sebuah **Font**, atau **Gaya** paragraf. Panel mencantumkan tampilan yang Anda pilih di bawah tombol. Pilih **Tanpa Pemformatan** untuk menghapusnya.

Anda dapat mengosongkan **Temukan apa**. Maka setiap bagian teks dengan tampilan tersebut menjadi kecocokan. Jika ada kata di **Temukan apa**, hanya kata dengan tampilan tersebut yang ditemukan. **Ganti Semua** hanya mengubah kata-kata tersebut.

## Persempit pencarian

- **Cocokkan huruf besar/kecil** hanya menemukan kata dengan huruf kapital yang sama. Jika pilihan ini aktif, "Plan" tidak menemukan "plan".
- **Temukan hanya kata utuh** melewati kata yang lebih panjang yang memuat kata Anda. Jika pilihan ini aktif, "plan" tidak menemukan "planet".
- **Cari** menentukan arah **Temukan Berikutnya** dari kursor. **Bawah** pergi ke akhir dokumen, dan **Atas** pergi ke awal. Masing-masing berhenti di sana dan memberi tahu Anda. **Semua** melanjutkan ke ujung lainnya dan merupakan pilihan yang umum. **Ganti Semua** selalu mengubah seluruh dokumen.

## Ganti kata

1. Tekan **Ctrl+H**, atau klik **Beranda > Pengeditan > Ganti**. Kursor berpindah ke **Ganti dengan**.
2. Ketikkan kata baru di **Ganti dengan**. Untuk menghapus kata yang ditemukan, biarkan kosong.
3. Ketikkan kata lama di **Temukan apa**.
4. Klik **Ganti** untuk melihat kecocokan pertama. Klik **Ganti** lagi untuk mengubahnya. JeffWrite kemudian pindah ke kecocokan berikutnya. Setiap perubahan adalah satu langkah, sehingga **Urungkan** (Ctrl+Z) membatalkan satu per satu.
5. Klik **Ganti Semua** untuk mengubah setiap kecocokan sekaligus. Panel memberi tahu Anda berapa banyak yang diubah. **Ganti Semua** juga mengubah kata dalam kotak teks dan bentuk, dan merupakan satu langkah, sehingga satu **Urungkan** mengembalikan semua kata. Jika **Lacak Perubahan** aktif, setiap penggantian dicatat sebagai perubahan yang dilacak, di teks maupun di kotak. Kotak yang terlalu besar untuk diubah dihitung dalam pesan. **Ganti Semua** tidak menjangkau kotak teks di header dan footer, bentuk yang dikelompokkan, atau kotak yang saling tertaut. **Temukan Berikutnya** belum mencari di dalam kotak teks.

Kata baru mengikuti huruf pertama dari kata yang diganti. Jika Anda mengganti kata yang dicetak tebal, kata baru juga dicetak tebal.

Misalnya, untuk mengubah "color" menjadi "colour" di seluruh dokumen: ketik **color** di **Temukan apa**, ketik **colour** di **Ganti dengan**, lalu klik **Ganti Semua**.

## Yang dicari oleh pencarian

- Pencarian memeriksa teks dokumen, termasuk teks dalam tabel. Penanda di tengah sebagian kata, yang dimiliki sebagian berkas, tidak menghalangi kata tersebut untuk ditemukan. **Ganti** dan **Ganti Semua** mempertahankan penanda tersebut.
- **Temukan Berikutnya** memeriksa teks lebih dulu. Kemudian dilanjutkan ke header, lalu footer, lalu catatan kaki, dan kembali ke awal teks. Jika kecocokan berada di salah satunya, JeffWrite membukanya untuk diedit dan memilih kecocokan tersebut. Panel memberi tahu lokasinya, misalnya "Ditemukan di header halaman 3." **Ganti** menukar kecocokan yang sedang Anda lihat, baik di teks maupun di header, footer, atau catatan kaki. **Ganti Semua** mengubah setiap kecocokan di semuanya, dan satu **Urungkan** mengembalikan semuanya. Catatan akhir merupakan bagian dari teks, sehingga ikut dicari bersama teks. Lihat [Header dan footer](headers-footers).
- Pencarian melewati teks yang diatur sebagai **Tersembunyi**, karena teks tersebut tidak ditampilkan. Lihat [Ubah tampilan teks](font).

## Terkait

- [Ubah tampilan teks](font)
- [Menggunakan JeffWrite dengan keyboard](keyboard)
