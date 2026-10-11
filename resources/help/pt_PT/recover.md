# Recuperar trabalho não guardado
<!-- keywords: recuperar, recuperação, recuperação automática, guardar automaticamente, gravação automática, falha, bloqueio, trabalho perdido, não guardado, falha de energia, corte de energia, restaurar, bloqueado, fechado por engano, cópia de segurança -->

Se o JeffPub fechar inesperadamente ou houver um corte de energia, o seu trabalho mais recente poderá estar a salvo. Enquanto trabalha, o JeffPub mantém uma cópia de reserva de cada publicação com alterações que ainda não guardou. Esta funcionalidade chama-se **Recuperação Automática**.

## Como funciona a Recuperação Automática

- De poucos em poucos minutos, o JeffPub guarda uma cópia de reserva de cada publicação com alterações não guardadas. A predefinição é a cada 10 minutos.
- O JeffPub mantém as cópias numa pasta de Recuperação Automática, dentro da pasta onde guarda os seus próprios dados. Nunca precisa de procurar lá.
- A cópia de reserva nunca substitui o seu ficheiro real.
- A cópia desaparece quando guarda a publicação, a fecha ou sai do JeffPub da forma normal.

A Recuperação Automática não substitui o ato de guardar. Se o JeffPub parar, pode perder o trabalho dos últimos minutos. Prima Ctrl+S com frequência.

## Recuperar o seu trabalho

Da próxima vez que iniciar o JeffPub após uma falha, a janela **Recuperar Trabalho Não Guardado** abre-se automaticamente. Cada linha mostra o nome da publicação, o local onde o ficheiro estava guardado (ou "nunca guardado") e quando a cópia foi feita.

1. Deixe uma marca de verificação junto de cada publicação que pretende recuperar. Desmarque as restantes.
2. Clique em **Abrir**.
3. Verifique o trabalho e, em seguida, prima Ctrl+S.

A publicação recuperada mostra "(Recuperado)" na barra de título até a guardar. Ctrl+S pede um nome e abre na pasta do ficheiro original, com o nome original. Escolha o nome original para substituir o ficheiro ou escolha um nome novo para manter ambos.

As imagens ligadas a ficheiros são encontradas tal como seriam se a publicação estivesse aberta na pasta do ficheiro original: as que estão nessa pasta, ou numa pasta dentro dela, são carregadas normalmente. Uma imagem ligada a um ficheiro noutro local mostra a pré-visualização guardada e fica marcada como **Não atualizada**; selecione-a e clique em **Atualizar Ligação** no [Gestor de Gráficos](graphics-manager). Uma publicação que nunca foi guardada não tem pasta, pelo que todas as imagens ligadas aguardam por **Atualizar Ligação**.

Os outros botões da janela são:

- **Eliminar** remove definitivamente as cópias marcadas, depois de pedir confirmação.
- **Agora não** fecha a janela e mantém as cópias. O JeffPub volta a oferecê-las da próxima vez que for iniciado.

O JeffPub só oferece cópias de uma sessão que terminou inesperadamente. Não oferece cópias de uma janela do JeffPub que ainda esteja aberta.

## Encontrar trabalho recuperado mais tarde

Se fechou a janela e pretende recuperar as cópias, clique em **Ficheiro > Abrir** e, em seguida, em **Recuperar Trabalho Não Guardado**. Se não houver nada para recuperar, o JeffPub informa-o.

## Alterar a frequência com que guarda

Clique em **Ficheiro > Opções** e, em seguida, no separador **Guardar**. Altere **Guardar informações de Recuperação Automática a cada** para um número entre 1 e 120 minutos. Um número mais pequeno significa menos trabalho a perder. O novo intervalo entra totalmente em vigor da próxima vez que iniciar o JeffPub.

## Relacionados

- [Abrir e guardar publicações](open-save)
- [Alterar as definições do JeffPub](options)
- [Obter ajuda e comunicar problemas](support)
