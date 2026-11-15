# Segurança: até onde um arquivo pode alcançar
<!-- keywords: central de confiabilidade, segurança, macros, arquivos vinculados, links, imagens vinculadas, mídia vinculada, internet, baixado, somente leitura, protegido, links externos, programas, seguro -->

Um arquivo pode apontar para outras coisas: arquivos, páginas da web, vídeos e até programas. A página **Segurança** é onde você decide o que o JeffWrite faz em relação a isso. Clique em **Arquivo > Opções** e, em seguida, na guia **Segurança**. Clique em **OK** para manter suas alterações ou em **Cancelar** para descartá-las.

Estas escolhas valem para todos os aplicativos do JeffOffice: se você alterar uma opção em um aplicativo, ela muda em todos. Uma linha referente a algo que o aplicativo não tem aparece como **Isto não se aplica** e fica em cinza.

## Macros

Macros são pequenos programas guardados dentro de um arquivo. O JeffWrite os mantém no arquivo, para que continuem lá quando você salvar, e **nunca os executa**. Não existe nenhuma opção que os execute.

## Arquivos vinculados e vínculos de pastas de trabalho

Isto não se aplica ao JeffWrite: ele não tem arquivos vinculados para atualizar. A linha fica em cinza.

## Vídeos e sons vinculados

Isto não se aplica ao JeffWrite: ele não tem vídeos ou sons vinculados para reproduzir. A linha fica em cinza.

## Arquivos da internet ou de outros computadores

Um arquivo que você baixou, ou que está em outro computador da sua rede, pode não ser seguro. Esta opção define como o JeffWrite o abre.

- **Abrir primeiro como somente leitura** é a escolha usual. O arquivo abre para que você possa lê-lo, mas não pode alterá-lo. Uma barra abaixo do arquivo informa isso. Clique em **Habilitar Edição** na barra para alterar apenas esse arquivo (até você fechar o programa). Quando você o salvar no mesmo lugar depois, a marca de download é removida do arquivo. Ou use **Salvar Como** para criar uma cópia, ou altere esta opção para a escolha seguinte.
- **Abrir normalmente** abre todos os arquivos do mesmo modo.

Um arquivo aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Somente Leitura** a partir da tela **Arquivo** é aberto desse modo, independentemente do que esta opção indicar.

## Links que iniciam um programa

Um link que iniciaria um programa ou um script (um arquivo com extensão .exe, .bat, .sh e assim por diante) é **sempre recusado**. O JeffWrite avisa você, e nada é executado. Um link para um arquivo em outro computador nunca é seguido sem perguntar antes. Nenhuma opção altera isso.

## Veja também

- [Alterar as configurações do JeffWrite](options)
- [Links](links)
