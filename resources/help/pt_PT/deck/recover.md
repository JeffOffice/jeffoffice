# Recuperar trabalho não guardado
<!-- keywords: recuperar, recuperação, recuperação automática, autorecover, guardar automaticamente, gravação automática, falha, bloqueio, trabalho perdido, não guardado, falha de energia, corte de energia, restaurar, bloqueado, fechado por engano, cópia de segurança -->

Se o JeffDeck fechar inesperadamente ou houver um corte de energia, o seu trabalho mais recente poderá estar a salvo. Enquanto trabalha, o JeffDeck mantém uma cópia de reserva de qualquer apresentação com alterações que ainda não guardou. Esta funcionalidade chama-se **Recuperação Automática**.

## Como funciona a Recuperação Automática

- A cada 10 minutos, o JeffDeck guarda uma cópia de reserva de cada apresentação com alterações não guardadas.
- O JeffDeck mantém as cópias numa pasta de Recuperação Automática, dentro da pasta onde guarda os seus próprios dados. Nunca precisa de procurar lá.
- A cópia de reserva nunca substitui o seu ficheiro real.
- A cópia desaparece quando guarda a apresentação, a fecha ou sai do JeffDeck da forma normal.

A Recuperação Automática não substitui o ato de guardar. Se o JeffDeck parar, pode perder o trabalho dos últimos minutos. Prima Ctrl+S com frequência.

## Recuperar o trabalho

Da próxima vez que iniciar o JeffDeck depois de uma falha, a janela **Recuperar Trabalho Não Guardado** abre-se automaticamente. Cada linha mostra o nome da apresentação, o local onde o ficheiro estava guardado (ou "nunca guardado") e quando a cópia foi criada.

1. Deixe uma marca de verificação junto a cada apresentação que pretende recuperar. Desmarque as restantes.
2. Clique em **Abrir**.
3. Verifique o trabalho e, em seguida, prima Ctrl+S.

Uma apresentação recuperada abre como trabalho não guardado. Ctrl+S pede um nome e começa na pasta do ficheiro original, com o nome original. Escolha o nome original para substituir o ficheiro ou escolha um nome novo para manter ambos.

Os outros botões da janela são:

- **Eliminar** remove definitivamente as cópias marcadas, depois de pedir confirmação.
- **Agora não** fecha a janela e mantém as cópias. O JeffDeck volta a propô-las da próxima vez que for iniciado.

O JeffDeck só propõe cópias de uma sessão que terminou inesperadamente. Não propõe cópias de uma janela do JeffDeck que ainda esteja aberta.

## Encontrar trabalho recuperado mais tarde

Se fechou a janela e quer recuperar as cópias, clique em **Ficheiro > Abrir** e, em seguida, em **Recuperar Trabalho Não Guardado**. Se não houver nada para recuperar, o JeffDeck indica-o.

## Relacionado

- [Abrir e guardar apresentações](open-save)
- [O ecrã Ficheiro](file-screen)
- [Alterar as definições do JeffDeck](options)
