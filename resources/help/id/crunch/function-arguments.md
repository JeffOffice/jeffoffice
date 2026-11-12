# Memasukkan fungsi dengan dialog Argumen Fungsi
<!-- keywords: argumen fungsi, argumen, fx, hasil rumus, Shift+F3, pilih sel, rentang, opsional, ulangi, edit fungsi, fungsi bersarang, kesalahan -->

Dialog **Argumen Fungsi** membantu Anda mengisi fungsi satu argumen per satu, dan menampilkan hasil rumus sebelum Anda memasukkannya ke dalam sel.

## Membukanya

- Pilih **Pilih Fungsi** pada tab Rumus (**Shift+F3**), atau klik **fx** di sebelah bilah rumus, lalu pilih fungsi. Dialog akan terbuka untuk fungsi tersebut.
- Untuk mengubah fungsi yang sudah ada dalam rumus, letakkan kursor di dalam tanda kurungnya (di dalam sel atau di bilah rumus), lalu tekan **Shift+F3** atau klik **fx**. Dialog akan terbuka untuk fungsi tersebut, dengan argumennya di dalam kotak. Dalam rumus seperti =SUM(A1, IF(B1>2, 5, 6)), kursor di dalam IF akan mengubah IF, dan kursor di dalam SUM tetapi di luar IF akan mengubah SUM.

## Mengisinya

Setiap argumen memiliki satu baris: namanya, kotak untuk mengetikkannya, dan di sebelah kanannya, hasil nilai argumen seperti sekarang, misalnya 12,5, "teks", TRUE, atau {1,2,3} untuk sekumpulan sel. Nama yang dicetak tebal wajib diisi; nama yang tidak tebal bersifat opsional dan boleh dikosongkan.

- **Tab** berpindah ke kotak berikutnya, dan **Shift+Tab** ke kotak sebelumnya.
- Baris di bawah kotak-kotak menjelaskan apa yang dilakukan fungsi, dan baris setelahnya menjelaskan argumen tempat Anda berada.
- Fungsi seperti SUM menerima sebanyak argumen yang Anda perlukan: saat Anda mengetik di kotak terakhir, kotak baru akan muncul.
- Tombol di sebelah kanan kotak memungkinkan Anda menunjuk sel. Dialog akan berubah menjadi strip kecil, dan Anda memilih sel pada lembar dengan mouse (kotak akan menampilkan alamatnya saat Anda menyeret). Klik tombol pada strip, atau tekan **Enter**, untuk kembali.

## Hasilnya

**Hasil rumus =** menampilkan hasil keseluruhan fungsi, sesaat setelah Anda berhenti mengetik. Tidak ada yang ditulis ke lembar sampai Anda menekan **OK**. Jika ada yang salah, baris di bawah hasil akan menjelaskannya dengan kata-kata: misalnya bahwa nilai tidak ditemukan, bahwa sesuatu dibagi nol, atau bahwa angka tidak diizinkan. Rumus yang terlalu lama untuk dihitung di sini tidak ditampilkan; tekan **OK** dan lembar akan menghitungnya.

**Bantuan untuk fungsi ini** membuka halaman ini.

## Menyelesaikan

Tekan **Enter** atau klik **OK** untuk memasukkan fungsi. Pada sel yang tidak sedang diisi, rumus akan ditulis ke dalam sel. Pada rumus yang sedang Anda ketik, fungsi akan disisipkan di posisi kursor, atau menggantikan fungsi yang Anda ubah, dan Anda dapat melanjutkan mengetik. Tekan **Esc** atau klik **Batal** dan tidak ada yang berubah.
