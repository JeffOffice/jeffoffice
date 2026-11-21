# Lacak perubahan, dan terima atau tolak perubahan
<!-- keywords: bandingkan, bandingkan dokumen, dua versi, apa yang berubah, lacak perubahan, perubahan yang dilacak, revisi, markup, penandaan, terima, tolak, terima semua, tolak semua, tampilan untuk tinjauan, markup sederhana, semua markup, tanpa markup, asli, tampilkan markup, balon, panel tinjauan, bilah perubahan, penyisipan, penghapusan, ctrl+shift+e, penulis, peninjau, nama pengguna, suntingan dari orang lain -->

**Lacak Perubahan** menyimpan catatan setiap suntingan yang Anda buat, sehingga Anda (atau orang lain) dapat melihat apa yang berubah dan memutuskan apakah akan mempertahankannya. Gunakan fitur ini saat Anda menyunting dokumen milik orang lain, atau dokumen yang akan ditinjau seseorang setelah Anda.

## Menyalakan dan mematikan Lacak Perubahan

Klik **Tinjau > Lacak Perubahan**, atau tekan **Ctrl+Shift+E**. Bilah status di bagian bawah jendela menunjukkan apakah fitur ini aktif. Klik kata-kata tersebut untuk membalik keadaannya. Pengaturan ini disimpan di dalam dokumen, sehingga tetap aktif ketika dokumen kembali dari orang lain.

Saat fitur ini aktif:

- **Yang Anda ketik** ditandai sebagai penyisipan.
- **Yang Anda hapus** tidak benar-benar dibuang. Teks itu tetap ada, dicoret, sehingga dapat dikembalikan. Teks yang Anda ketik sendiri ketika Lacak Perubahan aktif adalah satu-satunya pengecualian: menghapusnya langsung menghilangkannya, karena tidak ada orang lain yang pernah melihatnya.
- **Pemisah paragraf yang Anda buat** (Enter) ditandai sebagai disisipkan, dan yang Anda hapus (Backspace di awal paragraf) ditandai sebagai dihapus.
- **Perubahan tampilan** (tebal, font, ukuran, warna, perataan, indentasi, atau gaya) dicatat beserta tampilan teks sebelumnya.

Setiap perubahan disimpan bersama nama pembuatnya dan waktunya. Nama itu berasal dari **Berkas > Opsi**, di bawah **Nama pengguna**. Ubah di sana jika namanya salah.

Setiap suntingan merupakan satu langkah, sehingga **Ctrl+Z** membatalkannya seperti suntingan lainnya.

## Tampilan sebuah perubahan

Secara bawaan, setiap orang memiliki warna perubahannya sendiri. Orang pertama yang perubahannya ditampilkan dokumen mendapat warna pertama, orang berikutnya warna kedua, dan seterusnya. Seseorang mempertahankan warna yang sama selama dokumen masih terbuka. Teks yang disisipkan digarisbawahi dengan warna orang tersebut, dan teks yang dihapus dicoret. Teks yang dipindahkan berwarna hijau: dicoret ganda di tempat asalnya, dan digarisbawahi ganda di tempat tujuannya. Teks yang diubah formatnya ditebalkan. Sebuah bilah di margin kiri menandai setiap baris yang memiliki perubahan.

Anda dapat mengubah semua ini. Klik **Tinjau > Pelacakan > Opsi Pelacakan** untuk membuka kotak **Opsi Lacak Perubahan**. Di bawah **Markup**, setiap jenis perubahan memiliki **Tampilan** dan **Warna**:

- **Penyisipan**: garis bawah, garis bawah ganda, coret, tebal, miring, hanya warna, atau (tidak ada).
- **Penghapusan**: coret, coret ganda, tersembunyi, caret, atau hanya warna. Dengan **Tersembunyi**, kata yang dihapus tidak ditampilkan dalam baris. Dengan **Caret**, kata itu juga tidak ditampilkan, dan tanda ^ kecil menunjukkan tempatnya semula. Teks yang dipindahkan keluar juga tidak ditampilkan ketika Anda memilih **Balon** untuk penghapusan, tetapi tidak dalam kondisi lain.
- **Dipindahkan dari** dan **Dipindahkan ke**: tampilan dan warna untuk teks yang dipindahkan. Pilihan ini tersedia selama **Lacak pemindahan** dicentang. Jika Anda menghapus centangnya, teks yang dipindahkan ditampilkan sebagai penghapusan biasa dan penyisipan biasa.
- **Pemformatan**: tebal, hanya warna, atau (tidak ada). Jika Anda menghapus centang **Lacak pemformatan**, perubahan pemformatan tidak ditandai dalam teks.
- **Bilah perubahan**: batas kiri, batas kanan, batas luar (sisi kanan halaman ganjil dan sisi kiri halaman genap), atau (tidak ada), serta warna bilah.

Setiap kotak **Warna** menawarkan **Menurut penulis** (warna masing-masing orang) atau satu warna untuk semua orang. Pilihan ini milik Anda. Pengaturan ini disimpan di komputer ini, bukan di dalam dokumen, berlaku untuk setiap dokumen dan setiap jendela yang terbuka, dan setiap jendela berubah segera setelah Anda mengklik **OK**. Bilah digambar dengan cara yang sama di layar, dalam hasil cetak, dan dalam PDF setiap kali markup ditampilkan.

## Pilih yang Anda lihat: Tampilan untuk Tinjauan

Klik **Tinjau > Tampilan untuk Tinjauan**, lalu pilih salah satu:

- **Semua Markup** menampilkan setiap perubahan dalam teks, dengan bilah di margin.
- **Markup Sederhana** menampilkan dokumen seperti tampilannya jika semua perubahan diterima, dengan bilah di margin pada setiap baris yang memiliki perubahan.
- **Tanpa Markup** menampilkan dokumen seperti tampilannya jika semua perubahan diterima, tanpa apa pun yang lain.
- **Asli** menampilkan dokumen seperti sebelum ada perubahan apa pun: teks yang dihapus kembali muncul, dan teks yang disisipkan hilang.

Pilihan ini hanya mengubah apa yang Anda lihat. Pilihan ini tidak pernah mengubah dokumen, dan tidak disimpan bersama dokumen. Dokumen yang memiliki perubahan akan terbuka dengan **Semua Markup**.

**Tinjau > Tampilkan Markup** memilih jenis perubahan yang ditampilkan: **Penyisipan dan Penghapusan**, **Pemformatan**, dan **Komentar**. Di bawahnya, pilih cara teks yang dihapus ditampilkan. **Tampilkan Semua Revisi dalam Baris** menyimpan teks yang dihapus dalam teks, dicoret. **Tampilkan Revisi dalam Balon** mengeluarkan teks yang dihapus dari baris dan menempatkannya dalam balon di margin. **Tampilkan Hanya Komentar dan Pemformatan dalam Balon** menyimpan penyisipan dan penghapusan dalam teks.

## Menerima atau menolak perubahan

Menerima perubahan berarti menyimpannya secara permanen. Menolak perubahan berarti membatalkannya.

- **Tinjau > Terima** menerima perubahan tempat kursor berada (atau setiap perubahan dalam teks yang Anda pilih), lalu berpindah ke perubahan berikutnya. **Tinjau > Tolak** melakukan hal yang sama, dengan membatalkan perubahan.
- Klik panah di bawah **Terima** atau **Tolak** untuk pilihan lainnya: **Terima Semua Perubahan yang Ditampilkan**, **Terima Semua Perubahan**, dan **Terima Semua Perubahan dan Hentikan Pelacakan**; serta **Tolak Semua Perubahan yang Ditampilkan** dan **Tolak Semua Perubahan**. "Yang ditampilkan" berarti perubahan dari jenis yang diaktifkan oleh **Tampilkan Markup** (dan, di panel **Tinjauan**, peninjau yang Anda pilih).
- **Sebelumnya** dan **Berikutnya** pada grup **Perubahan** berpindah dari satu perubahan ke perubahan lain tanpa mengubah apa pun. Dengan kursor tepat di awal teks, header, atau catatan, **Berikutnya** menuju perubahan yang dimulai tepat di sana.

Setiap perintah ini merupakan satu langkah, sehingga **Ctrl+Z** mengembalikan dokumen persis seperti semula.

Menerima penghapusan menghilangkan teks secara permanen. Menolak penyisipan juga menghilangkan teks. Menerima penyisipan mempertahankan teks dan menghilangkan tanda bahwa teks itu baru. Menolak penghapusan mempertahankan teks dan menghilangkan coretnya. Menolak perubahan tampilan mengembalikan tampilan yang dimiliki teks sebelumnya. Teks yang dipindahkan merupakan sepasang perubahan, satu di tempat asalnya dan satu di tempat tujuannya, yang dihubungkan oleh nama pemindahan. Jika pasangan itu ditandai dengan nama yang sama, seperti yang disimpan program lain, menerima atau menolak salah satu dari keduanya berlaku untuk keduanya, sehingga teks benar-benar dipindahkan atau dikembalikan. Nama yang dipakai lebih dari sekali, atau hanya memiliki satu bagian, tidak menghubungkan apa pun: setiap bagian menjadi perubahan tersendiri.

## Panel Tinjauan

Klik **Tinjau > Panel Tinjauan** untuk membuka daftar perubahan dalam dokumen: jenisnya, pembuatnya, waktunya, dan katanya. Klik salah satu untuk menampilkannya dalam teks. Gunakan **Terima**, **Tolak**, **Sebelumnya**, dan **Berikutnya** di bawah daftar, atau pilih satu peninjau dari kotak di atasnya untuk hanya melihat perubahan orang tersebut.

## Perubahan dalam header, footer, catatan, dan tabel

Perubahan yang dibuat orang lain dalam header, footer, catatan kaki, atau catatan akhir ditampilkan dan didaftarkan seperti perubahan dalam teks, termasuk header atau footer yang tidak digunakan halaman mana pun (misalnya footer halaman genap ketika dokumen tidak memakai halaman ganjil dan genap yang berbeda): **Berikutnya** melewatinya, dan **Terima** serta **Tolak** melakukan perubahan yang terakhir Anda tuju di sana. **Berikutnya**, **Sebelumnya**, **Terima**, **Tolak**, dan panel **Tinjauan** membawa Anda ke header, footer, atau catatan yang memiliki perubahan, dan urutannya adalah teks lebih dulu, lalu catatan kaki, lalu header dan footer. **Terima Semua** dan **Tolak Semua** mencakup semuanya.

Perubahan pada tabel juga didaftarkan, baik dalam teks maupun dalam header dan footer: baris atau sel yang disisipkan atau dihapus, sel yang digabungkan, serta perubahan tampilan tabel, baris, atau sel. Bilah di margin menandai baris-baris dari baris atau sel tersebut. Menerima baris yang dihapus, atau menolak baris yang disisipkan, menghilangkan baris itu beserta isinya. Baris yang setiap selnya memiliki perubahan dihitung sebagai perubahan baris tersebut. Menolak perubahan tampilan mengembalikan tampilan yang dimiliki tabel, baris, atau sel, dan menolak penggabungan memisahkan kembali sel-selnya. Jika menghilangkan perubahan akan merusak tabel, perubahan itu dipertahankan dan pesan menyebutkan berapa banyak yang dipertahankan. Hal ini terjadi untuk sel yang digabungkan dengan sel lain ke bawah, yang tetap ada dan kehilangan tandanya, serta untuk perubahan kisi yang tidak dapat diikuti baris-barisnya, yang tetap ada sebagai perubahan yang dapat Anda terima. Perubahan di dalam tabel pada catatan kaki dipertahankan seperti ketika ditulis: perubahan itu tidak didaftarkan, dan Terima serta Tolak tidak memengaruhinya.

Pemindahan dihitung sebagai satu baris dalam daftar, dan satu perubahan dalam jumlah yang diberikan Terima Semua dan Tolak Semua: bagian lainnya dan tanda paragraf yang ikut dipindahkan ikut diselesaikan bersamanya. Perubahan di dalam kotak teks didaftarkan satu per satu, setelah perubahan dalam teks dan sebelum perubahan dalam catatan kaki. **Berikutnya** dan **Sebelumnya** membuka kotak untuk diketik, dengan perubahan itu terpilih, dan **Terima** serta **Tolak** menyelesaikan perubahan itu, masing-masing sebagai satu langkah. Memindahkan kursor di dalam kotak yang terbuka membuat **Berikutnya** dan **Sebelumnya** melanjutkan dari posisi kursor. Kotak yang diselesaikan satu per satu disimpan tanpa salinan yang dibaca program lama, seperti ketika Anda mengetik di dalam kotak. 200 kotak pertama yang memuat perubahan didaftarkan dengan cara ini. Kotak teks yang menjadi bagian dari kelompok, atau yang teksnya berlanjut ke kotak lain, kotak di dalam kotak lain, kotak di header atau catatan, serta kotak setelah 200 kotak pertama tidak dapat dibuka dengan cara ini: panel **Tinjauan** menghitung perubahan mereka dalam satu baris di bawah daftar, dan hanya **Terima Semua** dan **Tolak Semua** yang menanganinya.

Perubahan pada penomoran paragraf didaftarkan sebagai **Bernomor** atau **Penomoran diubah**. Ketika perubahan tampilan paragraf mencatat daftar dan level yang dimiliki paragraf, menolaknya akan mengembalikan paragraf ke daftar itu pada level tersebut (atau mengeluarkannya dari daftar yang tidak seharusnya ia miliki). Perubahan yang hanya menyimpan penomoran lama sebagai label (seperti yang dilakukan sebagian program) tidak menyebutkan daftar untuk dikembalikan: menerimanya mempertahankan penomoran, dan menolaknya menghilangkan penomoran hanya jika labelnya kosong; jika tidak, perubahan itu tetap ada dan dihitung sebagai dipertahankan. Menolak penomoran yang ditambahkan memberi paragraf apa yang diberikan gayanya (gaya judul yang memiliki penomoran tetap memilikinya), dan tanpa penomoran jika gayanya tidak memilikinya. Jika daftar yang akan dikembalikan oleh suatu perubahan sudah tidak ada dalam dokumen, tidak ada yang berubah pada paragraf itu, dan perubahan tersebut dihitung sebagai dipertahankan.

Perubahan pada bagian (ukuran halaman, orientasi, margin, kolom, penomoran halaman, atau jenis pemisah) didaftarkan sebagai **Bagian berformat**, pada paragraf yang mengakhiri bagian itu, atau di akhir teks untuk bagian terakhir. Menerimanya mempertahankan bagian sebagaimana adanya dan menghapus catatannya, sedangkan menolaknya mengembalikan properti yang dicatat, dan halaman langsung ditata ulang. Header dan footer yang dinamai bagian itu tidak berubah. Jika properti lama dalam catatan tidak dapat dibaca, menolak akan membiarkan bagian dan perubahannya apa adanya, dan dihitung sebagai dipertahankan; tidak ada yang diganti dengan nilai bawaan. Ketika Terima Semua atau Tolak Semua juga menghapus atau memulihkan pemisah bagian, perubahan setiap bagian diselesaikan lebih dulu, sehingga tidak ada yang tertinggal.

Perubahan yang tidak dapat diselesaikan tanpa merusak dokumen (tabel yang akan rusak, daftar atau bagian yang keadaan lamanya tidak dapat dikembalikan, atau kotak teks yang terlalu besar untuk ditulis kembali) dibiarkan seperti tertulis, dan pesan setelah **Terima Semua** atau **Tolak Semua** menyebutkan berapa banyak yang dipertahankan.

## Yang belum dilacak

Perubahan yang Anda buat sendiri pada tabel (baris, sel, penggabungan), pada penomoran dan daftar, pada bagian dan pengaturan halaman, serta pada teks di header, footer, dan catatan dilakukan seperti biasa, tanpa catatan. Perubahan yang dicatat program lain pada penomoran dan bagian didaftarkan dan dapat diterima atau ditolak, seperti dijelaskan di atas.

## Membandingkan dua versi

Bandingkan mencari perbedaan antara dua versi dokumen dan menampilkannya sebagai perubahan yang dilacak.

1. Buka versi yang lebih lama. Ini adalah **asli**.
2. Klik **Bandingkan** dalam grup **Bandingkan** pada tab **Tinjau**.
3. Klik **Telusuri** dan pilih versi yang lebih baru. Ini adalah versi **revisi**.
4. Ketikkan nama untuk menandatangani perubahan. Pilih pilihan lain yang Anda inginkan, misalnya mengabaikan huruf kapital. **Temukan teks yang dipindahkan** dan **Temukan perubahan pemformatan** aktif kecuali Anda menonaktifkannya.
5. Klik **OK**.

Jendela baru terbuka dengan teks asli dan perubahan yang mengubahnya menjadi teks revisi. Kedua berkas Anda tidak berubah. Gunakan **Terima** dan **Tolak** seperti pada perubahan yang dilacak lainnya. Jika Anda menerima semua perubahan, Anda mendapatkan teks revisi. Jika Anda menolak semua perubahan, Anda mendapatkan teks asli.

Bandingkan hanya memeriksa teks isi dokumen. Tabel, header, footer, dan catatan kaki tidak diperiksa.

Paragraf sepanjang 12 kata atau lebih yang dipotong dari satu tempat dan ditempelkan di tempat lain ditampilkan sebagai **pemindahan**: satu catatan di tempat teks itu berasal, dan satu di tempat ia tiba. Menerima atau menolak salah satu dari keduanya menghapus kedua catatan. Paragraf itu harus sama, atau setidaknya 9 dari 10 kata sama dan sebagian besar dalam urutan yang sama; jika tidak, paragraf itu ditampilkan sebagai penghapusan dan penyisipan. Kata yang berbeda di dalam paragraf yang dipindahkan ditampilkan sebagai penyisipan dan penghapusan di dalam pemindahan.

Teks yang sama di kedua versi tetapi tampak berbeda ditampilkan sebagai **perubahan pemformatan**. Bandingkan memeriksa tebal, miring, garis bawah, ukuran, font, warna, sorotan, serta gaya, perataan, indentasi, dan spasi paragraf. Menolak perubahan itu mengembalikan tampilan lama.

Jika kedua dokumen memiliki terlalu banyak perubahan, Bandingkan memberi tahu Anda. Bandingkan kemudian menampilkan beberapa paragraf sebagai diganti seluruhnya, dan mungkin tidak mencari pemindahan atau perubahan pemformatan di semuanya.
