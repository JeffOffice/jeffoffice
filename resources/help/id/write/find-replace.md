# Temukan dan ganti kata
<!-- keywords: temukan lanjutan, terdengar seperti, soundex, bentuk kata, semua bentuk kata, abaikan tanda baca, abaikan spasi, khusus, tanda paragraf, tab, temukan berdasarkan format, format, tebal, font, gaya, karakter pengganti, wildcard, pola, ekspresi reguler, grup, jumlah kecocokan, sorot semua kecocokan, ganti, ganti semua, cari, cari kata, ganti kata, cocokkan huruf besar kecil, kata utuh, temukan berikutnya, ctrl+f, ctrl+h, pengeditan, panel, teks tersembunyi, urungkan -->

Gunakan Temukan untuk melompat ke kata-kata dalam dokumen Anda. Gunakan Ganti untuk mengubahnya. Keduanya bekerja dalam satu panel di sebelah kanan jendela.

## Temukan kata

1. Tekan **Ctrl+F**, atau klik **Beranda > Pengeditan > Temukan**. Panel **Temukan dan Ganti** terbuka, dengan kursor berada di **Temukan apa**.
2. Ketik kata yang Anda inginkan. Lalu tekan **Enter**, atau klik **Temukan Berikutnya**.

JeffWrite mulai dari posisi kursor Anda. Program memilih tempat berikutnya yang memuat kata Anda, dan menggulir halaman agar Anda dapat melihatnya. Tempat itu disebut **kecocokan**. Klik **Temukan Berikutnya** lagi untuk pindah ke kecocokan berikutnya. Setelah kecocokan terakhir, pencarian kembali ke awal dokumen.

Saat Anda mengetik, panel menghitung kecocokan, misalnya "9 kecocokan", dan setiap kecocokan diberi pita kuning pada halaman. Pita tersebut juga tampil di sel tabel, header, footer, dan catatan kaki. Setelah **Temukan Berikutnya**, panel menyatakan kecocokan yang sedang Anda lihat, misalnya "Kecocokan 2 dari 4". Jumlah dan pita berubah saat Anda mengklik **Cocokkan huruf besar/kecil** atau **Temukan kata utuh saja**, atau saat Anda mengubah teks. Pita tetap ada di halaman ketika Anda menutup panel, dan saat Anda mengklik **Mode Baca**, sehingga Anda dapat membaca dengan semua kecocokan terlihat. Tidak ada yang berubah dalam dokumen, dan pita tidak pernah dicetak atau disimpan. Pita hilang saat Anda mengubah dokumen, saat Anda membuka panel lagi, atau saat Anda mengklik **Beranda > Pengeditan > Hapus Sorotan Pencarian**.

## Temukan dengan karakter pengganti

Centang **Gunakan wildcard** untuk mencari pola, bukan hanya kata. **Wildcard** adalah tanda yang mewakili huruf lain. Berikut tandanya:

- **?** mewakili satu huruf apa pun. **c?t** menemukan "cat" dan "cot".
- **\*** mewakili rangkaian huruf apa pun. **c\*t** menemukan "cat" dan "cart".
- **<** adalah awal kata, dan **>** adalah akhir kata. **<the>** menemukan "the" tetapi tidak "other".
- **[abc]** adalah salah satu huruf tersebut, **[a-c]** adalah huruf apa pun dari a sampai c, dan **[!abc]** adalah huruf selain itu.
- **{2}** mengulang huruf di depannya dua kali. **{1,3}** mengulangnya satu sampai tiga kali.
- **@** berarti satu atau lebih huruf di depannya.
- **\\** di depan tanda menemukan tanda itu sendiri. **\\?** menemukan tanda tanya.
- **^p** adalah tanda paragraf, **^t** adalah tab, **^#** adalah angka, **^$** adalah huruf, dan **^w** adalah spasi kosong.

Letakkan sebagian pola dalam **( )** untuk membuat **grup**. Dalam **Ganti dengan**, **\1** adalah grup pertama, **\2** adalah grup kedua, dan seterusnya sampai **\9**. **^&** adalah seluruh yang ditemukan. Misalnya, temukan **(<[A-Z]@>), (<[A-Z]@>)** dan ganti dengan **\2 \1** untuk mengubah "Smith, John" menjadi "John Smith". **Temukan kata utuh saja** dinonaktifkan ketika karakter pengganti aktif, karena **<** dan **>** sudah melakukan tugas itu.

Jika pola memiliki kesalahan, panel menyatakan jenisnya.

Jika kata Anda tidak ada dalam dokumen, panel memberitahukannya, dan tidak ada yang berpindah.

Anda dapat terus bekerja dalam dokumen saat panel terbuka. Klik halaman untuk mengetik. Klik **X** di bagian atas panel untuk menutupnya.

## Temukan dengan ekspresi reguler

Centang **Gunakan ekspresi reguler** untuk pola yang lebih rumit daripada karakter pengganti. **Ekspresi reguler** adalah pola singkat yang tersusun dari huruf dan tanda. Pilihan ini diaktifkan sebagai pengganti **Gunakan wildcard**: hanya satu dari keduanya yang dapat aktif. Berikut tanda yang umum digunakan:

- **.** adalah satu karakter apa pun, **\d** adalah angka, **\w** adalah huruf, angka, atau garis bawah, dan **\s** adalah spasi atau tab. **[a-c]** adalah huruf apa pun dari a sampai c, dan **[^abc]** adalah karakter selain itu.
- **^** adalah awal paragraf, **$** adalah akhir paragraf, dan **\b** adalah tepi kata. **\bcat\b** menemukan "cat" tetapi tidak "category."
- **\*** berarti nol atau lebih dari yang di depannya, **+** berarti satu atau lebih, dan **?** berarti nol atau satu. **{2}** berarti tepat dua kali, dan **{2,5}** berarti dua sampai lima kali; jumlah pengulangan dapat mencapai 200. Tambahkan **?** setelah pengulangan, seperti **.+?**, untuk menemukan rangkaian terpendek yang sesuai.
- **|** berarti "atau": **cat|dog** menemukan salah satu kata.
- **( )** membuat **grup**. Gunakan **(?: )** untuk grup yang tidak Anda perlukan dalam **Ganti dengan**.
- **\\** di depan tanda menemukan tanda itu sendiri: **\.** menemukan titik. **\t** adalah tab.

**Cocokkan huruf besar/kecil** bekerja seperti pada kata. Huruf dengan aksen dan huruf dari abjad lain dihitung sebagai huruf.

Pola dicari satu paragraf pada satu waktu, sehingga **^** dan **$** berarti awal dan akhir setiap paragraf. Untuk mencari melewati akhir paragraf, masukkan **\n** (tanda paragraf) dalam pola: **home\.\nLee** menemukan "home." di akhir satu paragraf dan "Lee" di awal paragraf berikutnya.

Dalam **Ganti dengan**, **$1** atau **\1** adalah grup pertama, **$2** atau **\2** adalah grup kedua, dan seterusnya sampai sembilan grup. **$0** atau **\0** adalah seluruh yang ditemukan. **$$** adalah tanda dolar, **\\** adalah garis miring terbalik, dan **\t** adalah tab. Misalnya, temukan **(\w+), (\w+)** dan ganti dengan **$2 $1** untuk mengubah "Smith, John" menjadi "John Smith."

**Ganti Semua** mengubah setiap kecocokan dalam teks, header, footer, catatan, dan kotak teks sebagai satu langkah, sehingga satu **Urungkan** mengembalikan semuanya. Saat Lacak Perubahan aktif, setiap perubahan dilacak, sama seperti untuk kata. Teks tersembunyi dibiarkan apa adanya.

Jika pola memiliki kesalahan, panel menyatakan di dekat karakter yang mana. Pola dapat tidak mencocokkan karakter apa pun, seperti **^** (awal paragraf) atau **$** (akhir): **Ganti dengan** kemudian memasukkan kata-katanya di tempat itu, sehingga **^** dengan **> ** menambahkan tanda tersebut di awal setiap paragraf. Pola yang akan memakan waktu terlalu lama pada teks Anda, seperti **(a+)+$** pada baris panjang berisi huruf a, dihentikan oleh JeffWrite, yang memberi tahu Anda; tidak ada yang diubah. Pencarian yang panjang menampilkan kotak **Mencari** dengan tombol **Batal**; membatalkan membiarkan dokumen Anda seperti semula, dan Anda tidak dapat mengetik dalam dokumen sampai pencarian selesai. Pola yang melihat ke depan atau ke belakang tanpa memakai apa yang diperiksanya, seperti **x(?=\d)**, dan pola dalam mode diperluas, seperti **(?x)**, tidak didukung; JeffWrite memberi tahu Anda. Paragraf yang sangat panjang dicari sebagian demi sebagian, dan setiap kecocokan adalah kecocokan yang akan ditemukan jika seluruh paragraf dicari. Pola yang mencocokkan rentang sangat panjang dalam paragraf yang sangat panjang (puluhan ribu karakter) dapat dihentikan dengan pesan "butuh pekerjaan terlalu banyak", dan satu kecocokan lebih dari 200.000 karakter tidak diganti: JeffWrite berhenti, memberi tahu Anda, dan tidak mengubah apa pun.

## Temukan lanjutan

Pilihan ini ada di panel di bawah **Gunakan ekspresi reguler**. Setiap pilihan mengubah cara JeffWrite membaca kata dalam **Temukan apa**.

- **Terdengar seperti** menemukan kata yang diucapkan dengan cara yang sama seperti kata Anda. Kata tersebut diberi kode dari bunyinya, lalu kata yang memiliki kode sama akan ditemukan. Ketik **Smith**, dan "Smyth" juga ditemukan.
- **Temukan semua bentuk kata** menemukan setiap bentuk dari kata tersebut. Ketik **run**, maka "runs," "running," dan "ran" ditemukan. Fitur ini bekerja pada kata utuh. Jika Anda mengetik dua kata, seperti **big dog**, keduanya ditemukan berdampingan, dalam bentuk apa pun.
- **Abaikan karakter tanda baca** melewati tanda seperti tanda hubung dan titik di antara huruf. Ketik **email**, maka "e-mail" juga ditemukan.
- **Abaikan karakter spasi kosong** melewati spasi dan tab di antara huruf. Ketik **e mail**, maka "email" ditemukan.

Hanya satu dari **Terdengar seperti**, **Temukan semua bentuk kata**, **Gunakan wildcard**, dan **Gunakan ekspresi reguler** yang dapat aktif pada satu waktu. **Terdengar seperti** dan **Temukan semua bentuk kata** selalu menemukan kata utuh, sehingga **Temukan kata utuh saja** dan **Cocokkan huruf besar/kecil** dinonaktifkan.

### Khusus

Klik **Khusus** untuk memasukkan kode dalam **Temukan apa**: **Tanda Paragraf** (^p), **Karakter Tab** (^t), **Angka Apa Pun** (^#), **Huruf Apa Pun** (^$), **Spasi Kosong** (^w), **Pemisah Bagian** (^b), **Bidang** (^d), **Tanda Catatan Kaki** (^f), **Tanda Catatan Akhir** (^e), atau **Karakter Caret** (^^). Kode-kode ini bekerja tanpa **Gunakan wildcard**. **Pemisah bagian** adalah tanda yang mengakhiri sebuah bagian. **Bidang** adalah bagian teks yang dihitung oleh program, seperti nomor halaman atau tanggal, dan **^d** menemukan tempat setiap bidang dimulai. Tanda catatan adalah angka kecil dalam teks yang menunjuk ke catatan kaki atau catatan akhir. Keempat tanda ini dapat ditemukan, tetapi **Ganti** dan **Ganti Semua** membiarkannya, karena mengubah salah satunya akan menghilangkan apa yang diwakilinya. Tanda lain dalam kotak, seperti **?** dan **\***, kemudian hanya berupa karakter tersebut.

### Format

Klik **Format** untuk hanya menemukan teks dengan tampilan tertentu: **Tebal**, **Miring**, **Garis Bawah**, **Coret**, **Disorot**, sebuah **Font**, atau **Gaya** paragraf. Panel mencantumkan tampilan yang Anda pilih di bawah tombol. Pilih **Tanpa Pemformatan** untuk menghapusnya.

Anda dapat mengosongkan **Temukan apa**. Maka setiap rangkaian teks dengan tampilan tersebut menjadi kecocokan. Jika ada kata dalam **Temukan apa**, hanya kata dengan tampilan itu yang ditemukan. **Ganti Semua** hanya mengubah kata-kata tersebut.

## Persempit pencarian

- **Cocokkan huruf besar/kecil** hanya menemukan kata dengan huruf kapital yang sama. Jika aktif, "Plan" tidak menemukan "plan."
- **Temukan kata utuh saja** melewati kata yang lebih panjang yang memuat kata Anda. Jika aktif, "plan" tidak menemukan "planet."
- **Cari** menentukan arah **Temukan Berikutnya** dari kursor. **Bawah** menuju akhir dokumen, dan **Atas** menuju awal. Setiap pilihan berhenti di sana dan memberi tahu Anda. **Semua** berlanjut memutar ke ujung lain dan merupakan pilihan yang biasa digunakan. **Ganti Semua** selalu mengubah seluruh dokumen.

## Ganti kata

1. Tekan **Ctrl+H**, atau klik **Beranda > Pengeditan > Ganti**. Kursor berpindah ke **Ganti dengan**.
2. Ketik kata baru dalam **Ganti dengan**. Untuk menghapus kata yang ditemukan, biarkan kosong.
3. Ketik kata lama dalam **Temukan apa**.
4. Klik **Ganti** untuk melihat kecocokan pertama. Klik **Ganti** lagi untuk mengubahnya. JeffWrite kemudian pindah ke kecocokan berikutnya. Setiap perubahan adalah satu langkah, sehingga **Urungkan** (Ctrl+Z) membatalkan satu per satu.
5. Klik **Ganti Semua** untuk mengubah setiap kecocokan sekaligus. Panel memberi tahu Anda berapa banyak yang diubah. **Ganti Semua** juga mengubah kata dalam kotak teks dan bentuk, dan merupakan satu langkah, sehingga satu **Urungkan** mengembalikan semua kata. Saat **Lacak Perubahan** aktif, setiap penggantian dicatat sebagai perubahan yang dilacak, dalam teks dan dalam kotak. Kotak yang terlalu besar untuk diubah dihitung dalam pesan. **Ganti Semua** tidak menjangkau kotak teks dalam header dan footer, bentuk yang dikelompokkan, atau kotak yang saling tertaut. **Temukan Berikutnya** belum mencari dalam kotak teks.

Kata baru mengikuti huruf pertama dari kata yang digantinya. Jika Anda mengganti kata tebal, kata baru juga dibuat tebal.

Misalnya, untuk mengubah "color" menjadi "colour" di seluruh dokumen: ketik **color** dalam **Temukan apa**, ketik **colour** dalam **Ganti dengan**, lalu klik **Ganti Semua**.

## Apa yang dicari

- Pencarian memeriksa teks dokumen, termasuk teks dalam tabel. Penanda di tengah beberapa kata, yang dimiliki sebagian berkas, tidak menghalangi kata tersebut untuk ditemukan. **Ganti** dan **Ganti Semua** mempertahankan penanda tersebut.
- **Temukan Berikutnya** mencari dalam teks terlebih dahulu. Kemudian dilanjutkan ke header, lalu footer, lalu catatan kaki, dan kembali ke awal teks. Jika kecocokan berada di salah satunya, JeffWrite membukanya untuk diedit dan memilih kecocokan tersebut. Panel memberi tahu Anda lokasinya, misalnya "Ditemukan di header halaman 3." **Ganti** menukar kecocokan yang sedang Anda lihat, di teks atau di header, footer, atau catatan kaki. **Ganti Semua** mengubah setiap kecocokan di semua tempat tersebut, dan satu **Urungkan** mengembalikan semuanya. Catatan akhir merupakan bagian dari teks, sehingga ikut dicari bersama teks. Lihat [Header dan footer](headers-footers).
- Pencarian melewati teks yang diatur sebagai **Tersembunyi**, karena teks itu tidak ditampilkan. Lihat [Ubah tampilan teks](font).

## Terkait

- [Ubah tampilan teks](font)
- [Gunakan JeffWrite dengan papan ketik](keyboard)
