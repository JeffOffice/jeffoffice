# Segurança: até onde um ficheiro pode chegar
<!-- keywords: centro de confiança, segurança, macros, ficheiros ligados, ligações, imagens ligadas, multimédia ligada, internet, transferidos, só de leitura, protegido, ligações externas, programas, seguro -->

Um ficheiro pode apontar para outras coisas: ficheiros, páginas Web, vídeos, até programas. A página **Segurança** é onde decide o que o JeffWrite faz em relação a isso. Clique em **Ficheiro > Opções** e, depois, no separador **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

Estas opções pertencem a todas as aplicações do JeffOffice: se alterar uma numa aplicação, a alteração é aplicada em todas. Uma linha para algo que uma aplicação não tem indica **Não se aplica** e fica a cinzento.

## Macros

As macros são pequenos programas guardados dentro de um ficheiro. O JeffWrite mantém-nas no ficheiro, para que continuem lá quando guardar, e **nunca as executa**. Não existe nenhuma definição que as execute.

## Ficheiros ligados e ligações a livros

Isto não se aplica ao JeffWrite: este programa não tem ficheiros ligados para atualizar. A linha fica a cinzento.

## Filmes e sons ligados

Isto não se aplica ao JeffWrite: este programa não tem filmes nem sons ligados para reproduzir. A linha fica a cinzento.

## Ficheiros da Internet ou de outros computadores

Um ficheiro que transferiu, ou que está num outro computador da sua rede, pode não ser seguro. Esta opção indica como o JeffWrite o abre.

- **Abrir primeiro como só de leitura** é a escolha habitual. O ficheiro abre-se para que o possa ler, mas não o pode alterar. Uma barra por baixo do ficheiro indica isso. Clique em **Ativar Edição** na barra para alterar apenas esse ficheiro (até fechar o programa). Quando o guardar depois no mesmo sítio, a marca de transferência é retirada do ficheiro. Ou use **Guardar Como** para criar uma cópia, ou altere esta definição para a escolha seguinte.
- **Abrir normalmente** abre todos os ficheiros da mesma forma.

Um ficheiro aberto com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Só de Leitura** a partir do ecrã **Ficheiro** abre-se dessa forma, independentemente do que esta opção indicar.

## Ligações que iniciam um programa

Uma ligação que iniciaria um programa ou um script (um ficheiro com a extensão .exe, .bat, .sh, e outras) é **sempre recusada**. O JeffWrite avisa-o e nada é executado. Uma ligação para um ficheiro noutro computador nunca é seguida sem que lhe seja pedida confirmação. Nenhuma definição altera isto.

## Consulte também

- [Alterar as definições do JeffWrite](options)
- [Ligações](links)
