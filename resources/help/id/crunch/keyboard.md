# Menggunakan JeffCrunch dengan keyboard
<!-- keywords: keyboard, pintasan, tombol, papan ketik, tombol panah, ctrl, shift, home, end, page up, page down, tab, enter, pilih semua, salin, F9, F1, zoom, tombol akses, tip tombol, sesuaikan keyboard, ubah tombol pintas, tetapkan ulang tombol, atur ulang tombol -->

Tab **Help** memiliki **Help** (F1), **Keyboard Shortcuts**, yang membuka halaman ini, **What's New**, dan **Feedback**, yang membuka halaman web masing-masing.

Pada Mac, tombol **Command** (⌘) berfungsi seperti **Ctrl** pada tabel di bawah, kecuali jika tabel menyatakan lain.

## Memindahkan dan memilih

| Tombol | Fungsinya |
| --- | --- |
| Tombol panah | Memindahkan sel aktif satu sel, dengan melewati baris dan kolom yang disembunyikan |
| Shift+panah | Memperluas pilihan satu sel |
| Ctrl+panah | Pergi ke tepi data (Esc menghentikan pencarian yang panjang) |
| Ctrl+Shift+panah | Memperluas pilihan sampai ke tepi data |
| End, lalu panah | Pergi ke tepi data (bilah status menampilkan End) |
| Ctrl+A | Memilih blok di sekitar sel aktif, lalu seluruh lembar (seluruh lembar sekaligus jika pilihan sudah lebih besar dari blok) |
| Ctrl+Shift+* | Memilih blok di sekitar sel aktif |
| Ctrl+klik atau Ctrl+seret | Menambahkan rentang ke pilihan |
| Ctrl+Shift+Enter | Memasukkan rumus sebagai rumus array pada sel yang dipilih |
| Home | Pergi ke kolom pertama pada baris |
| Ctrl+Home | Pergi ke sel pertama (A1, atau sel pertama yang ikut bergulir saat baris atau kolom dibekukan) |
| Ctrl+End | Pergi ke sel terakhir yang berisi sesuatu atau memiliki format |
| Page Down, Page Up | Pindah ke bawah atau ke atas satu layar |
| Tab, Shift+Tab | Pindah ke kanan atau ke kiri (melalui sel yang dipilih, jika ada beberapa) |
| Enter, Shift+Enter | Pindah ke bawah atau ke atas (melalui sel yang dipilih, jika ada beberapa) |
| Ctrl+Spasi | Memilih seluruh kolom |
| Shift+Spasi | Memilih seluruh baris |
| {key:SelectAll} | Memilih semua |

## Mengubah sel

| Tombol | Fungsinya |
| --- | --- |
| Karakter apa pun | Memulai entri di sel aktif, menggantikan isinya |
| F2 | Mengedit isi sel aktif di dalam sel; saat mengetik rumus, beralih antara menunjuk sel dan mengedit teks |
| F4 | Saat mengetik rumus, mengunci sebagian alamat pada posisi kursor dengan $ (A1, $A$1, A$1, $A1) |
| Enter, Shift+Enter | Menyimpan entri, lalu pindah ke bawah atau ke atas |
| Tab, Shift+Tab | Menyimpan entri, lalu pindah ke kanan atau ke kiri |
| Esc | Membatalkan entri |
| Alt+Enter | Memulai baris baru dalam sel |
| Delete | Menghapus isi sel yang dipilih |
| {key:Undo}, {key:Redo} | Urungkan, ulangi |
| {key:Save} | Simpan |
| {key:Cut}, {key:Copy}, {key:Paste} | Potong, salin, tempel (Esc menghilangkan bingkai sel yang disalin) |
| Ctrl+Alt+V | Tempel Khusus |
| Ctrl dan tombol plus, Ctrl+Shift dan tombol plus, Ctrl dan tombol minus | Menyisipkan, menghapus (baris atau kolom jika seluruh baris atau kolom dipilih; tombol plus menyisipkan sel yang dipotong, atau disalin dengan tidak ada perubahan sesudahnya, dan jika tidak, sel kosong) |
| Tombol panah, Shift+panah, Ctrl+panah (Option+panah pada Mac), Enter, Esc | Setelah Geser Catatan: memindahkan kotak catatan, mengubah ukurannya, membuat langkah 1 piksel, menyimpan, atau membatalkan |
| Ctrl+D, Ctrl+R | Isi ke bawah, isi ke kanan |
| Ctrl+E | Isi Kilat |
| Alt dan tanda sama dengan | Jumlah |
| Ctrl+0, Ctrl+9 | Menyembunyikan kolom, menyembunyikan baris |
| Alt+Down | Membuka daftar sel yang memilikinya |
| {key:Find} | Temukan |
| Ctrl+H (pada Mac, Option+Command+F) | Temukan dan Ganti |
| F5, Ctrl+G | Buka sel, rentang, atau nama |
| Ctrl+Shift+L | Menyalakan atau mematikan filter |
| Shift+F3 | Memilih fungsi |
| Ctrl+F3 | Daftar nama |
| F9, Shift+F9 | Hitung ulang semua, hitung ulang lembar ini |
| Tab, Enter | Saat rumus diketik: memilih fungsi atau nama yang ditandai dalam daftar |
| Esc | Menghentikan pencarian, pengurutan, atau filter yang sedang berjalan |

## Pilih tombol pintas Anda sendiri

Anda dapat mengubah tombol perintah apa pun, atau memberi tombol pada perintah yang belum memilikinya. Klik **File > Options**, lalu tab **Customize**.

1. Ketik di kotak **Search** untuk mencari perintah berdasarkan nama atau tombolnya. Daftar dikelompokkan menurut tab pita, dan setiap perintah menampilkan tombolnya. Perintah yang tidak ada di pita tercantum di bawah **Other**.
2. Klik perintah tersebut. Tombolnya tercantum di bawah **Keys for this command**.
3. Klik kotak **Press the new keys**, lalu tekan tombol yang Anda inginkan, misalnya **Ctrl+Shift+B**. Kemudian klik **Assign**. Sebuah perintah dapat memiliki hingga empat tombol.
4. Untuk menghapus tombol, klik tombol itu dalam daftar, lalu klik **Remove**.

Jika perintah lain sudah memakai tombol yang Anda tekan, JeffCrunch menyebutkan perintah tersebut. Klik **Take Over** untuk memberikan tombol itu kepada perintah ini; perintah lain akan kehilangan tombol tersebut. Tidak ada yang berubah sebelum Anda mengklik **OK**. Setelah itu, setiap jendela JeffCrunch yang terbuka menggunakan tombol Anda, begitu pula setiap jendela yang dibuka kemudian. Tooltip, menu, dan tabel pintasan di bawah menampilkan tombol Anda.

Simbol juga dapat memiliki tombol. Lihat **Sisipkan Simbol** di [Mengetik dalam sel](edit-cells): klik **Shortcut Key** pada kotak Symbol. Simbol itu kemudian tampil dalam daftar ini seperti perintah lainnya.

**Reset One** mengembalikan tombol bawaan perintah yang dipilih. **Reset All** mengembalikan tombol bawaan setiap perintah. JeffCrunch hanya menyimpan perintah yang tombolnya Anda ubah, sehingga perintah yang tidak pernah Anda ubah akan memperoleh tombol yang lebih baik dari versi yang lebih baru.

Beberapa tombol tidak dapat diberikan kepada perintah, karena sistem atau pengetikan membutuhkannya. JeffCrunch menampilkan **reserved** saat Anda menekan salah satunya. Tombol yang dicadangkan adalah **Ctrl+C**, **Ctrl+X**, **Ctrl+V**, **Ctrl+Z**, **Ctrl+A**, **Tab**, **Shift+Tab**, **Enter**, **Esc**, **Backspace**, **Delete**, **Alt+F4**, tombol panah, **Home**, **End**, **Page Up**, **Page Down**, dan **Spasi**, **F1**, **F6**, **Shift+F6**, **Ctrl+Y**, **Ctrl+Shift+Z**, **Ctrl** dengan tombol panah, **Home**, **End**, **Backspace**, atau **Delete** (dengan atau tanpa **Shift**, jika masuk akal dalam kotak teks), serta tombol huruf, angka, atau simbol yang ditekan sendiri atau dengan **Shift**. Tombol yang sudah dipakai bagian lain dari jendela juga dicadangkan. **Save**, **Undo**, dan **Redo** selalu mempertahankan setidaknya satu tombol. Pada Mac, **Ctrl** di sini berarti tombol **Command**.

## Perintah

Tabel ini menampilkan tombol yang telah Anda pilih.

<!-- shortcuts -->

## Tombol akses

Tekan **Alt** untuk melihat huruf pada setiap tab pita dan perintah, lalu tekan huruf-hurufnya. Tekan **Alt**, lalu **F**, untuk membuka layar File. Tekan **Esc** untuk kembali.
