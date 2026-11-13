# Tab Pemutaran
<!-- keywords: tab pemutaran, pita, film, video, suara, audio, putar, jeda, pratinjau, pangkas, keterangan, sisipkan keterangan, hapus keterangan, subtitel, webvtt, vtt, volume, bisukan, mulai, dalam urutan klik, otomatis, saat diklik, putar layar penuh, sembunyikan saat tidak diputar, ulangi hingga dihentikan, putar ulang setelah diputar, alat media -->

Tab **Pemutaran** mengatur cara film atau suara diputar, dan menambahkan keterangan ke film. Tab ini hanya tampil saat satu film atau suara dipilih, dan hilang saat Anda memilih objek lain.

- **Pratinjau** berisi **Putar**. Tombol ini memutar film atau suara pada slaid sehingga Anda dapat melihat dan mendengarnya. Selama diputar, tombol bertuliskan **Jeda**.
- **Pengeditan** berisi **Pangkas**. Tombol ini membuka kotak dengan **Waktu Mulai** dan **Waktu Selesai**, dalam detik. Film atau suara hanya diputar dari waktu mulai hingga waktu selesai. Bagian sisanya tetap berada di dalam presentasi. Untuk memutar sampai akhir berkas lagi, atur waktu selesai ke panjang penuhnya.
  - **Sisipkan Keterangan** menambahkan keterangan ke film dari berkas WebVTT (berkas teks yang berakhiran `.vtt`). Perintah ini meminta berkasnya, lalu bahasa keterangan, berupa kode singkat seperti `en`, `es`, atau `pt-BR`. Keterangan hanya dapat ditambahkan ke film yang tersimpan di dalam presentasi; untuk film yang hanya ditautkan dari luar, tombol ini redup. Sebuah film menyimpan satu set keterangan untuk setiap bahasa: menyisipkan berkas kedua untuk bahasa yang sama akan menggantikan yang pertama. Berkas yang bukan WebVTT, tidak memuat keterangan, atau terlalu besar (maksimal 4 MB, 20.000 keterangan, dan 2.000 karakter dalam satu keterangan) akan ditolak dengan pesan, dan tidak ada yang berubah.
  - **Hapus Keterangan** menghapus keterangan dari film. Jika film memiliki keterangan dalam lebih dari satu bahasa, program akan menanyakan keterangan mana yang akan dihapus, atau semuanya.
- **Opsi Media** mengatur cara film diputar dalam presentasi slaid:
  - **Volume** berisi **Rendah**, **Sedang**, **Tinggi**, atau **Bisukan**.
  - **Mulai** menentukan apa yang memulainya. **Dalam Urutan Klik** memutarnya pada sebuah klik, bersama animasi slaid. **Otomatis** memutarnya sendiri saat gilirannya tiba. **Saat Diklik** memutarnya saat Anda mengklik film atau suara.
  - **Putar Layar Penuh** membuat film memenuhi layar selama diputar. Pilihan ini hanya berlaku untuk film.
  - **Sembunyikan Saat Tidak Diputar** menyembunyikan gambar hingga film atau suara diputar, dan menyembunyikannya lagi setelah berhenti. Gambar pengeras suara pada suara disembunyikan sepanjang waktu.
  - **Ulangi hingga Dihentikan** memutarnya lagi setiap kali selesai, hingga dihentikan atau slaid berganti.
  - **Putar Ulang setelah Diputar** mengembalikan film ke gambar pertamanya saat selesai. Tanpa pilihan ini, film tetap berada pada gambar terakhir.

Keterangan tampil di bagian bawah film, berupa teks putih pada pita gelap, saat film diputar dalam presentasi slaid dan di editor. Keterangan diatur waktunya mengikuti film, dan ukurannya mengikuti ukuran film. Jika film memiliki lebih dari satu bahasa, bahasa pertama yang ditampilkan. Keterangan disimpan bersama presentasi sebagai bagian dari berkas `.pptx`, sehingga tetap ada saat Anda membuka kembali berkas. Berkas keterangan dengan format lain (bukan WebVTT) disimpan apa adanya dan disimpan ulang tanpa perubahan, tetapi tidak ditampilkan. Setelah sebuah film memiliki keterangan, **Periksa Aksesibilitas** tidak lagi mencantumkannya.

Setiap perubahan dihitung sebagai satu langkah. **Ctrl+Z** membatalkannya.

Suara atau film yang diputar lintas slaid, yang diatur oleh beberapa berkas, akan terus diputar selama jumlah slaid yang ditentukan berkas. JeffDeck memutarnya seperti itu dalam presentasi slaid, dan menyimpan pengaturan tersebut saat Anda menyimpan.

Lihat [Bentuk, gambar, tabel, dan Teks Seni](objects) dan [Memutar presentasi slaid dan menggunakan Tampilan Presenter](show).

## Terkait

- [Bentuk, gambar, tabel, dan Teks Seni](objects)
- [Tab Sisipkan](tab-insert)
- [Memutar presentasi slaid dan menggunakan Tampilan Presenter](show)
