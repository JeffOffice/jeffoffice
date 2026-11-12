# Gunakan JeffCrunch dengan keyboard
<!-- keywords: keyboard, pintasan, tombol, tombol pintas, panah, ctrl, shift, home, end, page up, page down, tab, enter, pilih semua, salin, F9, F1, zoom, tombol akses, tip tombol, sesuaikan keyboard, ubah tombol pintas, tetapkan ulang tombol, atur ulang tombol -->

Tab **Help** memiliki **Help** (F1), **Keyboard Shortcuts**, yang membuka halaman ini, **What's New**, dan **Feedback**, yang membuka halaman web masing-masing.

Di Mac, tombol **Command** (⌘) berfungsi seperti **Ctrl** pada tabel di bawah, kecuali jika tabel menyebutkan lain.

## Memindahkan dan memilih

| Tombol | Fungsinya |
| --- | --- |
| Tombol panah | Memindahkan sel aktif satu sel, melewati baris dan kolom yang disembunyikan |
| Shift+panah | Memperluas pilihan satu sel |
| Ctrl+panah | Pergi ke tepi data (Esc menghentikan pencarian yang panjang) |
| Ctrl+Shift+panah | Memperluas pilihan sampai ke tepi data |
| Home | Pergi ke kolom pertama pada baris |
| Ctrl+Home | Pergi ke sel pertama (A1, atau sel pertama yang ikut bergulir saat baris atau kolom dibekukan) |
| Ctrl+End | Pergi ke sel terakhir yang berisi sesuatu atau memiliki format |
| Page Down, Page Up | Turun atau naik satu layar |
| Tab, Shift+Tab | Pindah ke kanan atau kiri (melalui sel yang dipilih, jika ada beberapa) |
| Enter, Shift+Enter | Pindah ke bawah atau atas (melalui sel yang dipilih, jika ada beberapa) |
| Ctrl+Spasi | Memilih seluruh kolom |
| Shift+Spasi | Memilih seluruh baris |
| {key:SelectAll} | Memilih semuanya |

## Mengubah sel

| Tombol | Fungsinya |
| --- | --- |
| Karakter apa pun | Memulai entri di sel aktif, menggantikan isinya |
| F2 | Mengedit isi sel aktif di dalam sel; saat mengetik rumus, beralih antara menunjuk sel dan mengedit teks |
| F4 | Saat mengetik rumus, mengunci bagian alamat di kursor dengan $ (A1, $A$1, A$1, $A1) |
| Enter, Shift+Enter | Menyimpan entri, lalu pindah ke bawah atau atas |
| Tab, Shift+Tab | Menyimpan entri, lalu pindah ke kanan atau kiri |
| Esc | Membatalkan entri |
| Alt+Enter | Memulai baris baru di dalam sel |
| Delete | Menghapus isi sel yang dipilih |
| {key:Undo}, {key:Redo} | Urungkan, ulangi |
| {key:Save} | Simpan |
| {key:Cut}, {key:Copy}, {key:Paste} | Potong, salin, tempel (Esc menghilangkan bingkai sel yang disalin) |
| Ctrl+Alt+V | Tempel Khusus |
| Ctrl dan tanda plus, Ctrl dan tanda minus | Menyisipkan, menghapus (baris atau kolom jika seluruh baris atau kolom dipilih) |
| Ctrl+D, Ctrl+R | Isi ke bawah, isi ke kanan |
| Ctrl+E | Isi Kilat |
| Alt dan tanda sama dengan | Jumlahkan |
| Ctrl+0, Ctrl+9 | Menyembunyikan kolom, menyembunyikan baris |
| Alt+Bawah | Membuka daftar sel yang memilikinya |
| {key:Find} | Temukan |
| Ctrl+H (di Mac, Option+Command+F) | Temukan dan Ganti |
| F5, Ctrl+G | Pergi ke sel, rentang, atau nama |
| Ctrl+Shift+L | Mengaktifkan atau menonaktifkan filter |
| Shift+F3 | Memilih fungsi |
| Ctrl+F3 | Daftar nama |
| F9, Shift+F9 | Hitung ulang semua, hitung ulang lembar ini |
| Tab, Enter | Saat rumus sedang diketik: memilih fungsi atau nama yang ditandai dalam daftar |
| Esc | Menghentikan pencarian, pengurutan, atau filter yang sedang berjalan |

## Pilih tombol pintas Anda sendiri

Anda dapat mengubah tombol pada perintah apa pun, atau memberi tombol kepada perintah yang belum memilikinya. Klik **File > Options**, lalu tab **Customize**.

1. Ketik di kotak **Search** untuk mencari perintah berdasarkan namanya atau tombolnya. Daftar dikelompokkan menurut tab pita, dan setiap perintah menampilkan tombolnya. Perintah yang tidak ada di pita dicantumkan di bawah **Other**.
2. Klik perintah tersebut. Tombolnya tercantum di bawah **Keys for this command**.
3. Klik kotak **Press the new keys**, lalu tekan tombol yang Anda inginkan, misalnya **Ctrl+Shift+B**. Kemudian klik **Assign**. Sebuah perintah dapat memiliki hingga empat tombol.
4. Untuk menghapus tombol, klik tombol itu dalam daftar, lalu klik **Remove**.

Jika perintah lain sudah memakai tombol yang Anda tekan, JeffCrunch memberi tahu perintah yang mana. Klik **Take Over** untuk memberikan tombol itu kepada perintah ini, dan perintah lain akan kehilangannya. Tidak ada yang berubah sebelum Anda mengklik **OK**. Setelah itu, setiap jendela JeffCrunch yang terbuka menggunakan tombol Anda, begitu pula setiap jendela yang dibuka nanti. Tooltip, menu, dan tabel pintasan di bawah menampilkan tombol Anda.

Sebuah simbol juga dapat memiliki tombol. Lihat **Sisipkan Simbol** di [Mengetik dalam sel](edit-cells): klik **Shortcut Key** pada kotak Symbol. Simbol itu kemudian tampil dalam daftar ini seperti perintah lainnya.

**Reset One** mengembalikan tombol standar perintah yang dipilih. **Reset All** mengembalikan tombol standar semua perintah. JeffCrunch hanya menyimpan perintah yang tombolnya Anda ubah, sehingga perintah yang tidak pernah Anda ubah akan mendapatkan tombol yang lebih baik dari versi berikutnya.

Beberapa tombol tidak dapat diberikan kepada perintah, karena sistem atau pengetikan memerlukannya. JeffCrunch menampilkan **reserved** saat Anda menekan salah satunya. Tombol yang dicadangkan adalah **Ctrl+C**, **Ctrl+X**, **Ctrl+V**, **Ctrl+Z**, **Ctrl+A**, **Tab**, **Shift+Tab**, **Enter**, **Esc**, **Backspace**, **Delete**, **Alt+F4**, tombol panah, **Home**, **End**, **Page Up**, **Page Down**, dan **Spasi**, **F1**, **F6**, **Shift+F6**, **Ctrl+Y**, **Ctrl+Shift+Z**, **Ctrl** dengan tombol panah, **Home**, **End**, **Backspace**, atau **Delete** (dengan atau tanpa **Shift**, jika masuk akal dalam kotak teks), serta tombol huruf, angka, atau simbol yang ditekan sendiri atau dengan **Shift**. Tombol yang sudah dipakai bagian lain dari jendela juga dicadangkan. **Save**, **Undo**, dan **Redo** selalu mempertahankan setidaknya satu tombol. Di Mac, **Ctrl** di sini berarti tombol **Command**.

## Perintah

Tabel ini menampilkan tombol yang telah Anda pilih.

<!-- shortcuts -->

## Tombol akses

Tekan **Alt** untuk melihat huruf pada setiap tab pita dan perintah, lalu tekan hurufnya. Tekan **Alt**, lalu **F**, untuk membuka layar File. Tekan **Esc** untuk kembali.
