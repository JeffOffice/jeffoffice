# Segurança: a que podem aceder os ficheiros

<!-- keywords: centro de confiança, segurança, macros, ficheiros ligados, ligações, imagens ligadas, multimédia ligada, internet, transferido, só de leitura, protegido, ligações externas, programas, seguro, confiança, atualizar ligações -->

Um ficheiro pode apontar para outras coisas: ficheiros, páginas Web, vídeos e até programas. A página **Segurança** é onde decide o que o JeffCrunch faz em relação a isso. Clique em **Ficheiro > Opções** e, depois, no separador **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

Estas escolhas aplicam-se a todas as aplicações do JeffOffice: se alterar uma numa aplicação, a alteração é aplicada em todas. Uma linha para algo que uma aplicação não tem aparece com a indicação **Não se aplica** e a cinzento.

## Macros

As macros são pequenos programas guardados dentro de um ficheiro. O JeffCrunch mantém-nas no ficheiro, para que continuem lá quando guardar, e **nunca as executa**. Não existe nenhuma definição que as execute.

## Ficheiros ligados e ligações de livros

Trata-se dos outros ficheiros que um livro lê, como os livros de onde obtém números, designados **ligações**, os ficheiros que as suas consultas leem e os ficheiros a partir dos quais as suas tabelas dinâmicas são criadas. A escolha define o que o JeffCrunch faz quando abre um livro que lê alguns destes ficheiros e quando utiliza **Atualizar** ou **Atualizar Tudo** nas respetivas consultas e tabelas dinâmicas.

- **Perguntar primeiro** é a escolha habitual. Antes de o JeffCrunch ler um desses ficheiros, mostra o caminho completo do ficheiro e pergunta. Clique em **Ler o Ficheiro** apenas se confiar na origem do livro. **Não Ler**, o botão predefinido, não lê nada e deixa o livro como estava. Esta escolha pergunta sempre que abre um livro e, depois, uma vez por cada ficheiro. Depois de clicar em **Ler o Ficheiro** para um ficheiro, o JeffCrunch não volta a perguntar por esse mesmo ficheiro nessa janela, até fechar a janela.
- **Atualizar sempre** lê um ficheiro sem perguntar apenas se já tiver confirmado que esse mesmo livro lê esse mesmo ficheiro. Para todos os outros ficheiros, incluindo qualquer ficheiro indicado num livro que alguém lhe tenha acabado de enviar, continua a perguntar primeiro, da mesma forma.
- **Nunca atualizar** não lê nada quando um livro abre e não mostra nenhuma pergunta. Os números que o ficheiro guardou mantêm-se como estão. Quando clicar em **Atualizar** por iniciativa própria, isso é um pedido, por isso o JeffCrunch mostra então a pergunta. Depois de clicar em **Ler o Ficheiro**, não volta a perguntar por esse mesmo ficheiro nessa janela.

## O que fica memorizado

Quando clica em **Ler o Ficheiro** ou **Ler os Ficheiros**, o JeffCrunch memoriza essa associação: este livro e aquele ficheiro, identificados pelo respetivo caminho completo. Só a associação fica memorizada. Se o livro for movido ou renomeado, editado para ler outro ficheiro, ou se o ficheiro for substituído por um atalho para outro ficheiro, o JeffCrunch volta a perguntar. Um livro que nunca tenha guardado não tem caminho, por isso nada sobre ele fica memorizado depois de o fechar.

As associações ficam guardadas apenas nas suas próprias definições, neste computador, e nunca num livro. Assim, um livro não consegue acrescentar nenhuma, e quem receber um livro de si não recebe as associações. As definições guardam uma forma codificada de cada caminho, e não o próprio caminho. São guardadas até 2000 associações; quando há mais, esquece-se primeiro a associação usada há mais tempo.

Em **Opções > Segurança** pode ver quantas associações estão memorizadas (**Aprovações memorizadas**). Clique em **Esquecer Tudo** para as esquecer todas de uma só vez. Nenhum livro é alterado, e cada um volta a perguntar na próxima vez.

Um livro que indique "nunca" no seu próprio **Pedido ao iniciar** (**Dados > Editar Ligações**) fica sempre intacto. Um livro guardado noutro computador nunca é lido quando um ficheiro abre, seja qual for a sua escolha. Pode continuar a utilizar **Atualizar Valores** em **Editar Ligações** sempre que quiser.

## Vídeos e sons ligados

Isto não se aplica no JeffCrunch, que não tem vídeos nem sons ligados para reproduzir. A linha está a cinzento.

## Ficheiros da Internet ou de outros computadores

Um ficheiro que transferiu, ou que se encontra noutro computador da sua rede, pode não ser seguro. Esta escolha define a forma como o JeffCrunch o abre.

- **Abrir primeiro como só de leitura** é a escolha habitual. O ficheiro abre para que o possa ler, mas não o pode alterar. Uma barra por baixo do ficheiro indica isso. Clique em **Ativar Edição** na barra para alterar apenas esse ficheiro (até fechar o programa). Quando o guardar no mesmo local, a marca de transferência é retirada do ficheiro. Em alternativa, utilize **Guardar Como** para criar uma cópia, ou altere esta definição para a escolha seguinte.
- **Abrir normalmente** abre todos os ficheiros da mesma forma.

Um ficheiro que abra com **Abrir Como Cópia**, **Abrir e Reparar** ou **Abrir Só para Leitura** a partir do ecrã **Ficheiro** abre dessa forma, seja qual for a indicação desta opção.

## Ligações que iniciam um programa

Uma ligação que iniciaria um programa ou um script (um ficheiro com a extensão .exe, .bat, .sh e semelhantes) é **sempre recusada**. O JeffCrunch avisa-o, e nada é executado. Uma ligação para um ficheiro noutro computador nunca é seguida sem que lhe seja pedida confirmação primeiro. Nenhuma definição altera isto.

## Consulte também

- [Alterar as definições do JeffCrunch](options)
- [Notas e ligações](notes-links)
