# Segurança: até onde pode um ficheiro chegar
<!-- keywords: centro de confiança, segurança, macros, ficheiros ligados, ligações, imagens ligadas, multimédia ligada, internet, transferido, só de leitura, protegido, ligações externas, programas, seguro -->

Um ficheiro pode apontar para outras coisas: ficheiros, páginas Web, vídeos e até programas. A página **Segurança** é onde decide o que o JeffPub faz em relação a isso. Clique em **Ficheiro > Opções** e, depois, no separador **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

Estas opções pertencem a todas as aplicações do JeffOffice: se alterar uma numa aplicação, a alteração aplica-se em todas. Uma linha referente a algo que a aplicação não tem indica **Não se aplica** e aparece a cinzento.

## Macros

As macros são pequenos programas guardados dentro de um ficheiro. O JeffPub **nunca as executa**, e não existe nenhuma definição que as execute. O JeffPub também não mantém as macros de uma publicação: ao guardar um ficheiro `.pub` que abriu, a cópia não terá macros, por isso guarde o original se ainda precisar delas.

## Ficheiros ligados e ligações de livros

Isto diz respeito a imagens que estão **ligadas** a um ficheiro, em vez de estarem guardadas dentro da publicação. A escolha define o que o JeffPub faz quando abre uma publicação que contenha algumas.

- **Atualizar** é a opção habitual. Quando uma publicação é aberta, o JeffPub volta a ler o ficheiro de cada imagem ligada, se o ficheiro estiver na pasta da própria publicação, para que a imagem fique atualizada. Nunca pergunta.
- **Nunca atualizar** mantém cada imagem tal como foi guardada. Cada uma mostra **Não atualizada** até usar **Atualizar Ligação** nela. **Exportar para PDF** e **Imprimir** indicam então, na barra de estado, que foram usadas pré-visualizações nas imagens que não têm uma cópia completa na publicação.

O JeffPub só lê um ficheiro que esteja na pasta da própria publicação (ou numa pasta que tenha indicado como aceitável). Qualquer outro ficheiro ligado aguarda até que o peça.

## Multimédia ligada

Isto não se aplica no JeffPub, que não tem vídeos nem sons ligados para reproduzir. A linha aparece a cinzento.

## Ficheiros da Internet ou de outros computadores

Um ficheiro que transferiu, ou que está noutro computador da sua rede, pode não ser seguro. Esta opção define a forma como o JeffPub o abre.

- **Abrir primeiro como só de leitura** é a opção habitual. O ficheiro abre-se para que o possa ler, mas não o pode alterar. Uma barra por baixo do ficheiro indica isso. Clique em **Ativar Edição** na barra para alterar apenas esse ficheiro (até fechar o programa). Quando o guardar no mesmo local, o JeffPub retira a marca de transferência do ficheiro. Em alternativa, use **Guardar Como** para criar uma cópia, ou altere esta definição para a opção seguinte.
- **Abrir normalmente** abre todos os ficheiros da mesma forma.

Um ficheiro aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Só para Leitura** a partir do ecrã **Ficheiro** abre-se dessa forma, seja qual for a definição.

## Ligações que iniciam um programa

Uma ligação que iniciaria um programa ou um script (um ficheiro com a extensão .exe, .bat, .sh, e assim por diante) é **sempre recusada**. O JeffPub avisa-o e nada é executado. Uma ligação para um ficheiro noutro computador nunca é seguida sem que o JeffPub lho pergunte primeiro. Nenhuma definição altera isto.

## Consulte também

- [Alterar as definições do JeffPub](options)
- [Imagens](pictures)
