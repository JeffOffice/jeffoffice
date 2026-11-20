# Segurança: até onde um arquivo pode acessar
<!-- keywords: central de confiabilidade, segurança, macros, arquivos vinculados, vínculos, imagens vinculadas, mídia vinculada, internet, baixado, somente leitura, protegido, links externos, programas, seguro -->

Um arquivo pode apontar para outras coisas: arquivos, páginas da web, vídeos e até programas. A página **Segurança** é onde você decide o que o JeffPub faz com isso. Clique em **Arquivo > Opções** e, em seguida, na guia **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

Essas escolhas valem para todos os aplicativos do JeffOffice: se você alterar uma em um aplicativo, ela muda em todos. Uma linha para algo que o aplicativo não tem mostra **Isto não se aplica** e fica esmaecida.

## Macros

Macros são pequenos programas guardados dentro de um arquivo. O JeffPub **nunca as executa**, e não há nenhuma configuração que as execute. O JeffPub também não mantém as macros de uma publicação: quando você salva um arquivo `.pub` que abriu, a cópia não tem nenhuma. Por isso, guarde o original se ainda precisar delas.

## Arquivos vinculados e vínculos de pasta de trabalho

Isto se refere a imagens **vinculadas** a um arquivo, em vez de guardadas dentro da publicação. A escolha define o que o JeffPub faz ao abrir uma publicação que tenha alguma delas.

- **Atualizar automaticamente** é a opção usual. Ao abrir uma publicação, o JeffPub lê de novo o arquivo de cada imagem vinculada se ele estiver na pasta da própria publicação, assim a imagem fica atualizada. Ele nunca pergunta.
- **Nunca atualizar** mantém cada imagem como foi salva. Cada uma mostra **Não atualizada** até você usar **Atualizar Vínculo** nela. **Exportar para PDF** e **Imprimir** informam na barra de status que visualizações foram usadas para imagens que não têm uma cópia completa na publicação.

O JeffPub lê apenas arquivos que estejam na pasta da própria publicação (ou em uma pasta que você tenha autorizado). Qualquer outro arquivo vinculado aguarda até que você peça.

## Vídeos e sons vinculados

Isto não se aplica no JeffPub, que não tem vídeos ou sons vinculados para reproduzir. A linha fica esmaecida.

## Arquivos da internet ou de outros computadores

Um arquivo que você baixou, ou que está em outro computador da sua rede, pode não ser seguro. Esta escolha define como o JeffPub o abre.

- **Abrir primeiro como somente leitura** é a opção usual. O arquivo abre para leitura, mas não pode ser alterado. Uma barra abaixo do arquivo informa isso. Clique em **Habilitar Edição** na barra para alterar somente esse arquivo (até você fechar o programa). Quando você salvá-lo no mesmo lugar depois, o JeffPub remove a marca de download do arquivo. Ou use **Salvar Como** para criar uma cópia, ou altere esta configuração para a opção seguinte.
- **Abrir normalmente** abre todos os arquivos da mesma forma.

Um arquivo aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir como Somente Leitura** na tela **Arquivo** abre desse modo, seja qual for a configuração.

## Links que iniciam um programa

Um link que iniciaria um programa ou um script (um arquivo com extensão .exe, .bat, .sh e similares) é **sempre recusado**. O JeffPub avisa você, e nada é executado. Um link para um arquivo em outro computador nunca é seguido sem perguntar antes. Nenhuma configuração altera isso.

## Veja também

- [Alterar as configurações do JeffPub](options)
- [Imagens](pictures)
