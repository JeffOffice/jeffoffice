# Segurança: até onde um arquivo pode acessar

<!-- keywords: central de confiabilidade, segurança, macros, arquivos vinculados, links, imagens vinculadas, mídia vinculada, internet, baixado, somente leitura, protegido, links externos, programas, seguro, central de segurança -->

Um arquivo pode apontar para outras coisas: arquivos, páginas da web, vídeos e até programas. A página **Segurança** é onde você decide o que o JeffDeck faz com isso. Clique em **Arquivo > Opções** e, em seguida, na guia **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

Essas escolhas valem para todos os aplicativos do JeffOffice: se você alterar uma em um aplicativo, ela muda em todos. Uma linha de algo que o aplicativo não tem exibe **Não se aplica** e aparece esmaecida.

## Macros

Macros são pequenos programas guardados dentro de um arquivo. O JeffDeck os mantém no arquivo, para que continuem lá quando você salvar, e **nunca os executa**. Não existe nenhuma configuração que os execute.

## Arquivos vinculados e vínculos de pastas de trabalho

Isto não se aplica no JeffDeck: ele não tem arquivos vinculados para atualizar. A linha fica esmaecida.

## Filmes e sons vinculados

Isto diz respeito a um filme ou som que um slide **vincula** em vez de manter dentro da apresentação.

- **Nunca reproduzir** é a escolha habitual. Um filme ou som vinculado informa que está vinculado e não é reproduzido.
- **Perguntar antes** pergunta uma vez, para cada arquivo vinculado, na primeira vez em que você o reproduzir.
- **Reproduzir** reproduz sem perguntar.

Mesmo com **Reproduzir**, só é reproduzido um arquivo de vídeo ou som cujo nome esteja neste computador. Um endereço da web, um arquivo em outro computador, um programa, um link ou arquivo especial, e um arquivo que não seja realmente um vídeo ou som nunca são reproduzidos. Um arquivo vinculado pode ter até 256 MB. O arquivo é lido em segundo plano por até 20 segundos, para que um disco lento não congele o JeffDeck. Uma unidade de rede que parece local (uma unidade mapeada, ou uma pasta em /Volumes ou /mnt) é lida como um arquivo local, por isso pode ser lenta. Com **Perguntar antes**, uma apresentação de slides pergunta uma vez, ao começar, sobre todos os arquivos vinculados nela.

## Arquivos da internet ou de outros computadores

Um arquivo que você baixou, ou que está em outro computador da sua rede, pode não ser seguro. Esta opção define como o JeffDeck o abre.

- **Abrir primeiro como somente leitura** é a escolha habitual. O arquivo abre para que você possa lê-lo, mas não pode alterá-lo. Uma barra sob o arquivo informa isso. Clique em **Habilitar Edição** na barra para alterar somente esse arquivo (até você fechar o programa). Quando você salvar nele depois, a marca de download é removida do arquivo. Ou use **Salvar Como** para criar uma cópia, ou altere esta configuração para a opção seguinte.
- **Abrir normalmente** abre todos os arquivos da mesma forma.

Um arquivo aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Somente Leitura** a partir da tela **Arquivo** abre dessa forma, independentemente desta configuração.

## Links que iniciam um programa

Um link que iniciaria um programa ou um script (um arquivo com extensão .exe, .bat, .sh e assim por diante) é **sempre recusado**. O JeffDeck avisa você, e nada é executado. Um link para um arquivo em outro computador nunca é seguido sem que você seja perguntado antes. Nenhuma configuração altera isso.

## Veja também

- [Alterar as configurações do JeffDeck](options)
- [Executar a apresentação de slides](show)
