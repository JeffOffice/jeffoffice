# Segurança: até onde pode um ficheiro chegar
<!-- keywords: centro de confiança, segurança, macros, ficheiros ligados, ligações, imagens ligadas, multimédia ligada, internet, transferido, só de leitura, protegido, ligações externas, programas, seguro -->

Um ficheiro pode apontar para outras coisas: ficheiros, páginas Web, vídeos e até programas. A página **Segurança** é onde decide o que o JeffDeck faz em relação a isso. Clique em **Ficheiro > Opções** e, em seguida, no separador **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

Estas opções pertencem a todas as aplicações do JeffOffice: se alterar uma numa aplicação, a alteração aplica-se em todas. Uma linha referente a algo que uma aplicação não tem indica **Não se aplica** e fica a cinzento.

## Macros

As macros são pequenos programas guardados dentro de um ficheiro. O JeffDeck mantém-nas no ficheiro, pelo que continuam lá quando guarda, e **nunca as executa**. Não existe nenhuma definição que as execute.

## Ficheiros ligados e ligações a livros

Isto não se aplica no JeffDeck, pois este programa não tem ficheiros ligados para atualizar. A linha fica a cinzento.

## Vídeos e sons ligados

Isto diz respeito a um vídeo ou som que um diapositivo **liga** em vez de o conter dentro da apresentação.

- **Nunca reproduzir** é a escolha habitual. Um vídeo ou som ligado indica que está ligado e não é reproduzido.
- **Perguntar primeiro** pergunta uma vez por cada ficheiro ligado, na primeira vez que o reproduzir.
- **Reproduzir** reproduz sem perguntar.

Mesmo com **Reproduzir**, só é reproduzido um ficheiro de vídeo ou de som cujo nome esteja neste computador. Um endereço Web, um ficheiro noutro computador, um programa, uma ligação ou um ficheiro especial, e um ficheiro que não seja realmente um vídeo ou um som, nunca são reproduzidos. Um ficheiro ligado pode ter até 256 MB. O ficheiro é lido em segundo plano durante até 20 segundos, para que uma unidade lenta não bloqueie o JeffDeck. Uma unidade de rede que pareça local (uma unidade mapeada ou uma pasta em /Volumes ou /mnt) é lida como um ficheiro local, pelo que pode ser lenta. Com **Perguntar primeiro**, uma apresentação de diapositivos pergunta uma só vez, ao iniciar, por todos os ficheiros ligados que contém.

## Ficheiros da Internet ou de outros computadores

Um ficheiro que transferiu, ou que está noutro computador da sua rede, pode não ser seguro. Esta opção indica como o JeffDeck o abre.

- **Abrir primeiro como só de leitura** é a escolha habitual. O ficheiro abre para que o possa ler, mas não o pode alterar. Uma barra por baixo do ficheiro indica isso. Clique em **Ativar Edição** na barra para alterar apenas esse ficheiro (até fechar o programa). Quando o guardar no mesmo local, a marca de transferência é removida do ficheiro. Ou use **Guardar Como** para criar uma cópia, ou altere esta definição para a opção seguinte.
- **Abrir normalmente** abre todos os ficheiros da mesma forma.

Um ficheiro que abra com **Abrir como Cópia**, **Abrir e Reparar** ou **Abrir Só de Leitura** a partir do ecrã **Ficheiro** abre dessa forma, independentemente do que esta opção indique.

## Ligações que iniciam um programa

Uma ligação que iniciaria um programa ou um script (um ficheiro com a extensão .exe, .bat, .sh, e assim por diante) é **sempre recusada**. O JeffDeck informa-o e nada é executado. Uma ligação para um ficheiro noutro computador nunca é seguida sem que lhe seja perguntado primeiro. Nenhuma definição altera isto.

## Consulte também

- [Alterar as definições do JeffDeck](options)
- [Executar a apresentação de diapositivos](show)
