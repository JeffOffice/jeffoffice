# Segurança: até onde um arquivo pode acessar

<!-- keywords: central de confiabilidade, segurança, macros, arquivos vinculados, vínculos, imagens vinculadas, mídia vinculada, internet, baixado, somente leitura, protegido, links externos, programas, seguro -->

Um arquivo pode apontar para outras coisas: arquivos, páginas da web, vídeos e até programas. A página **Segurança** é onde você decide o que o JeffPub faz com isso. Clique em **Arquivo > Opções** e, em seguida, na guia **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

Estas escolhas valem para todos os aplicativos do JeffOffice: se você mudar uma em um aplicativo, ela muda em todos. Uma linha para algo que um aplicativo não tem mostra **Não se aplica** e fica em cinza.

## Macros

Macros são pequenos programas guardados dentro de um arquivo. O JeffPub os mantém no arquivo, para que continuem lá quando você salvar, e **nunca os executa**. Não existe nenhuma configuração que os execute.

## Arquivos vinculados e vínculos de pastas de trabalho

Isto se refere a imagens que estão **vinculadas** a um arquivo, em vez de guardadas dentro da publicação. A escolha define o que o JeffPub faz ao abrir uma publicação que tenha algumas delas.

- **Atualizar** é a opção habitual. Quando uma publicação é aberta, o JeffPub lê novamente o arquivo de cada imagem vinculada se ele estiver na pasta da própria publicação, para que a imagem fique atualizada. Ele nunca pergunta.
- **Nunca atualizar** mantém cada imagem como foi salva. Cada uma mostra **Não atualizada** até que você use **Atualizar Vínculo** nela. **Exportar para PDF** e **Imprimir** então informam na barra de status que visualizações foram usadas para imagens que não têm uma cópia completa na publicação.

O JeffPub só lê um arquivo que esteja na pasta da própria publicação (ou em uma pasta que você tenha indicado como confiável). Qualquer outro arquivo vinculado aguarda até que você peça.

## Mídia vinculada

Isto não se aplica ao JeffPub, pois ele não tem vídeos ou sons vinculados para reproduzir. A linha fica em cinza.

## Arquivos da internet ou de outros computadores

Um arquivo que você baixou, ou que está em outro computador da sua rede, pode não ser seguro. Esta escolha define como o JeffPub o abre.

- **Abrir como somente leitura primeiro** é a opção habitual. O arquivo abre para que você possa lê-lo, mas não pode alterá-lo. Uma barra sob o arquivo informa isso. Clique em **Habilitar Edição** na barra para alterar somente esse arquivo (até você fechar o programa). Quando você salvá-lo no mesmo lugar depois, o JeffPub remove a marca de download do arquivo. Ou use **Salvar Como** para criar uma cópia, ou mude esta configuração para a próxima opção.
- **Abrir normalmente** abre todos os arquivos do mesmo jeito.

Um arquivo aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir como Somente Leitura** a partir da tela **Arquivo** abre desse modo, independentemente do que esta configuração disser.

## Links que iniciam um programa

Um link que iniciaria um programa ou um script (um arquivo com extensão .exe, .bat, .sh e assim por diante) é **sempre recusado**. O JeffPub avisa você, e nada é executado. Um link para um arquivo em outro computador nunca é seguido sem que você seja consultado antes. Nenhuma configuração altera isso.

## Veja também

- [Alterar as configurações do JeffPub](options)
- [Imagens](pictures)
