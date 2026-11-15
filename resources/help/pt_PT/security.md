# Segurança: até onde um ficheiro pode aceder
<!-- keywords: centro de confiança, segurança, macros, ficheiros ligados, ligações, imagens ligadas, multimédia ligada, internet, transferido, só de leitura, protegido, ligações externas, programas, seguro -->

Um ficheiro pode apontar para outras coisas: ficheiros, páginas web, vídeos, e até programas. A página **Segurança** é onde decide o que o JeffPub faz com isso. Clique em **Ficheiro > Opções** e, depois, no separador **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

Estas opções pertencem a todas as aplicações do JeffOffice: se alterar uma numa aplicação, ela muda em todas. Uma linha para algo que a aplicação não tem indica **Não se aplica** e aparece a cinzento.

## Macros

As macros são pequenos programas guardados dentro de um ficheiro. O JeffPub mantém-nas no ficheiro, para que continuem lá ao guardar, e **nunca as executa**. Não existe nenhuma definição que as execute.

## Ficheiros ligados e ligações a livros

Isto diz respeito a imagens que estão **ligadas** a um ficheiro em vez de estarem guardadas dentro da publicação. A escolha define o que o JeffPub faz quando abre uma publicação que tenha algumas.

- **Atualizar** é a opção habitual. Quando uma publicação é aberta, o JeffPub volta a ler o ficheiro de cada imagem ligada, se o ficheiro estiver na pasta da própria publicação, para que a imagem fique atualizada. Nunca pergunta.
- **Nunca atualizar** deixa todas as imagens tal como foram guardadas. Cada uma indica **Não atualizada** até usar **Atualizar Ligação** nela. **Exportar para PDF** e **Imprimir** informam então na barra de estado que foram usadas pré-visualizações para imagens que não têm uma cópia completa na publicação.

O JeffPub só lê um ficheiro que esteja na pasta da própria publicação (ou numa pasta que tenha indicado como segura). Qualquer outro ficheiro ligado aguarda até que o peça.

## Multimédia ligada

Isto não se aplica no JeffPub: o programa não tem vídeos nem sons ligados para reproduzir. A linha está a cinzento.

## Ficheiros da Internet ou de outros computadores

Um ficheiro que tenha transferido, ou que esteja noutro computador da sua rede, pode não ser seguro. Esta opção indica como o JeffPub o abre.

- **Abrir primeiro como só de leitura** é a opção habitual. O ficheiro abre para que o possa ler, mas não o pode alterar. Uma barra por baixo do ficheiro indica isso. Clique em **Ativar Edição** na barra para alterar apenas esse ficheiro (até fechar o programa). Quando o guardar no próprio local, o JeffPub remove a marca de transferência do ficheiro. Ou use **Guardar Como** para criar uma cópia, ou mude esta definição para a opção seguinte.
- **Abrir normalmente** abre todos os ficheiros da mesma forma.

Um ficheiro que abra com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Só de Leitura** a partir do ecrã **Ficheiro** abre dessa forma, seja qual for a definição.

## Ligações que iniciam um programa

Uma ligação que iniciasse um programa ou um script (um ficheiro com a extensão .exe, .bat, .sh e semelhantes) é **sempre recusada**. O JeffPub avisa-o e nada é executado. Uma ligação para um ficheiro noutro computador nunca é seguida sem que lho pergunte primeiro. Nenhuma definição altera isto.

## Consulte também

- [Alterar as definições do JeffPub](options)
- [Imagens](pictures)
