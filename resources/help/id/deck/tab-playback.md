# Tab Pemutaran
<!-- keywords: tab pemutaran, pita, film, video, suara, audio, putar, jeda, pratinjau, pangkas, keterangan, sisipkan keterangan, hapus keterangan, subtitle, webvtt, vtt, volume, bisukan, mulai, dalam urutan klik, otomatis, saat diklik, putar layar penuh, sembunyikan saat tidak diputar, ulangi hingga dihentikan, putar ulang setelah diputar, putar lintas slaid, teks alternatif, rata, tinggi, lebar, ukuran, alat media -->

Tab **Pemutaran** mengatur cara film atau suara diputar, dan menambahkan keterangan ke film. Tab ini hanya tampil saat satu film atau suara dipilih, dan hilang saat Anda memilih yang lain.

- **Pratinjau** berisi **Putar**. Tombol ini memutar film atau suara pada slaid agar Anda dapat melihat dan mendengarnya. Selama diputar, tombol berubah menjadi **Jeda**.
- **Pengeditan** berisi **Pangkas**. Tombol ini membuka kotak dengan **Waktu Mulai** dan **Waktu Selesai**, dalam detik. Film atau suara hanya diputar dari waktu mulai hingga waktu selesai. Bagian lainnya tetap berada di dalam presentasi. Untuk memutar sampai akhir berkas lagi, atur waktu selesai ke panjang penuhnya.
  - **Sisipkan Keterangan** menambahkan keterangan ke film dari berkas WebVTT (berkas teks yang berakhiran `.vtt`). Perintah ini meminta berkas, lalu bahasa keterangan sebagai kode singkat, misalnya `en`, `es`, atau `pt-BR`. Keterangan hanya dapat ditambahkan ke film yang tersimpan di dalam presentasi; untuk film yang hanya ditautkan dari luar, tombol ini redup. Sebuah film menyimpan satu set keterangan untuk setiap bahasa: menyisipkan berkas kedua untuk bahasa yang sama akan menggantikan berkas pertama. Berkas yang bukan WebVTT, tidak memuat keterangan, atau terlalu besar (maksimum 4 MB, 20.000 keterangan, dan 2.000 karakter dalam satu keterangan) akan ditolak dengan pesan, dan tidak ada yang berubah.
  - **Hapus Keterangan** menghapus keterangan dari film. Jika film memiliki keterangan dalam lebih dari satu bahasa, program akan menanyakan keterangan mana yang akan dihapus, atau semuanya.
- **Opsi Media** mengatur cara film atau suara diputar dalam presentasi slaid:
  - **Volume** berisi **Rendah**, **Sedang**, **Tinggi**, atau **Bisukan**.
  - **Mulai** menentukan apa yang memulai pemutaran. **Dalam Urutan Klik** memutarnya saat diklik, bersama animasi slaid. **Otomatis** memutarnya sendiri saat gilirannya tiba. **Saat Diklik** memutarnya saat Anda mengklik film atau suara.
  - **Putar Layar Penuh** membuat film memenuhi layar saat diputar. Fitur ini hanya berlaku untuk film.
  - **Sembunyikan Saat Tidak Diputar** menyembunyikan gambar hingga film atau suara diputar, dan menyembunyikannya lagi setelah berhenti. Gambar ikon pengeras suara untuk suara selalu disembunyikan.
  - **Ulangi hingga Dihentikan** memutarnya lagi setiap kali selesai, hingga dihentikan atau slaid berganti.
  - **Putar Ulang setelah Diputar** mengembalikan film ke gambar pertamanya saat selesai. Tanpa opsi ini, film tetap berhenti pada gambar terakhir.
  - **Putar Lintas Slaid** (untuk suara; film hanya ditampilkan pada slaidnya sendiri) membuat suara terus diputar dalam presentasi slaid saat slaid berganti, hingga selesai atau dihentikan, sampai akhir presentasi. Berkas dari program lain mungkin menyatakan berapa slaid suara itu diputar lintas: jumlah tersebut tetap ada selama kotak dicentang, dan suara berhenti setelah sejumlah slaid itu, dihitung dari slaid tempat suara dimulai (berpindah bolak-balik di antara slaid tersebut tidak mengurangi jumlahnya). Jika tidak dicentang, suara berhenti bersama slaidnya; jika dicentang lagi, suara berlanjut hingga akhir presentasi. Dengan **Ulangi hingga Dihentikan**, suara dimulai lagi setiap kali selesai. Tekan Ctrl+Z untuk membatalkan perubahan, dan berkas .pptx menyimpannya.

Keterangan tampil di bagian bawah film, dengan teks putih pada pita gelap, saat film diputar dalam presentasi slaid dan di editor. Keterangan diatur waktunya mengikuti film, dan ukurannya mengikuti ukuran film. Jika film memiliki lebih dari satu bahasa, bahasa pertama yang ditampilkan. Keterangan disimpan bersama presentasi sebagai bagian dari berkas `.pptx`, sehingga muncul kembali saat Anda membukanya lagi. Berkas keterangan dengan format lain (bukan WebVTT) tetap dipertahankan dan disimpan kembali tanpa perubahan, tetapi tidak ditampilkan. Setelah film memiliki keterangan, **Periksa Aksesibilitas** tidak lagi mencantumkannya.

Grup **Aksesibilitas**, **Atur**, dan **Ukuran** sama dengan yang ada pada tab Format Gambar: **Teks Alternatif**, **Rata**, serta kotak **Tinggi** dan **Lebar**, untuk film atau suara yang dipilih.

Setiap perubahan dihitung sebagai satu langkah. **Ctrl+Z** membatalkannya.

Suara atau film yang diputar lintas slaid, yang diatur oleh sebagian berkas, akan terus diputar selama jumlah slaid yang ditentukan berkas. JeffDeck memutarnya dengan cara itu dalam presentasi slaid, dan menyimpan pengaturan tersebut saat Anda menyimpan.

Lihat [Bentuk, gambar, tabel, dan Seni Teks](objects) dan [Putar presentasi slaid dan gunakan Tampilan Presenter](show).

## Terkait

- [Bentuk, gambar, tabel, dan Seni Teks](objects)
- [Tab Sisipkan](tab-insert)
- [Putar presentasi slaid dan gunakan Tampilan Presenter](show)
