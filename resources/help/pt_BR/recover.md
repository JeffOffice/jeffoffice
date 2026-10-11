# Recuperar trabalho não salvo
<!-- keywords: recuperar, recuperação, autorecuperação, auto recuperação, salvamento automático, salvar automaticamente, travamento, falha, trabalho perdido, não salvo, queda de energia, falta de luz, restaurar, travado, congelado, fechado por engano, fechou sem querer, backup, cópia de segurança -->

Se o JeffPub fechar inesperadamente ou faltar energia, seu trabalho mais recente ainda pode estar seguro. Enquanto você trabalha, o JeffPub mantém uma cópia reserva de cada publicação com alterações não salvas. Esse recurso se chama **AutoRecuperação**.

## Como a AutoRecuperação funciona

- A cada poucos minutos, o JeffPub salva uma cópia reserva de cada publicação com alterações não salvas. O padrão é a cada 10 minutos.
- O JeffPub guarda as cópias em uma pasta de AutoRecuperação, dentro da pasta onde mantém seus próprios dados. Você nunca precisa procurar nela.
- A cópia reserva nunca substitui o seu arquivo de verdade.
- A cópia é removida quando você salva a publicação, fecha a publicação ou sai do JeffPub normalmente.

A AutoRecuperação não substitui o salvamento. Se o JeffPub parar, você pode perder o trabalho dos últimos minutos. Pressione Ctrl+S com frequência.

## Recuperar seu trabalho

Na próxima vez que você iniciar o JeffPub depois de uma falha, a janela **Recuperar Trabalho Não Salvo** será aberta automaticamente. Cada linha mostra o nome da publicação, o local onde o arquivo estava guardado (ou "nunca salvo") e quando a cópia foi feita.

1. Deixe marcada a caixa ao lado de cada publicação que você deseja recuperar. Desmarque as outras.
2. Clique em **Abrir**.
3. Confira o trabalho e pressione Ctrl+S.

A publicação recuperada mostra "(Recuperada)" na barra de título até ser salva. Ao pressionar Ctrl+S, o JeffPub pede um nome e começa na pasta do arquivo original, com o nome original. Escolha o nome original para substituir o arquivo ou escolha um novo nome para manter os dois.

As imagens vinculadas a arquivos são localizadas como se a publicação estivesse aberta na pasta do arquivo original: as que estão nessa pasta, ou em uma pasta dentro dela, são carregadas normalmente. Uma imagem vinculada a um arquivo em qualquer outro local mostra a visualização salva e fica marcada como **Não atualizada**; selecione-a e clique em **Atualizar Vínculo** no [Gerenciador de Elementos Gráficos](graphics-manager). Uma publicação que nunca foi salva não tem pasta, então todas as suas imagens vinculadas aguardam **Atualizar Vínculo**.

Os outros botões da janela são:

- **Excluir** remove definitivamente as cópias marcadas, depois de pedir confirmação.
- **Agora não** fecha a janela e mantém as cópias. O JeffPub as oferece novamente na próxima vez que for iniciado.

O JeffPub só oferece cópias de uma sessão que terminou inesperadamente. Ele não oferece cópias de uma janela do JeffPub que ainda está aberta.

## Encontrar o trabalho recuperado mais tarde

Se você fechou a janela e quer suas cópias de volta, clique em **Arquivo > Abrir** e depois em **Recuperar Trabalho Não Salvo**. Se não houver nada para recuperar, o JeffPub informará isso.

## Alterar a frequência de salvamento

Clique em **Arquivo > Opções** e depois na guia **Salvar**. Altere **Salvar informações de AutoRecuperação a cada** para um número de 1 a 120 minutos. Um número menor significa menos trabalho a perder. O novo intervalo passa a valer totalmente na próxima vez que você iniciar o JeffPub.

## Tópicos relacionados

- [Abrir e salvar publicações](open-save)
- [Alterar as configurações do JeffPub](options)
- [Obter ajuda e relatar problemas](support)
