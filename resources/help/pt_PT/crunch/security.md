# Segurança: até onde um ficheiro pode aceder
<!-- keywords: centro de confiança, segurança, macros, ficheiros ligados, ligações, imagens ligadas, multimédia ligada, internet, transferido, só de leitura, protegido, ligações externas, programas, seguro -->

Um ficheiro pode apontar para outras coisas: ficheiros, páginas Web, vídeos e até programas. A página **Segurança** é onde decide o que o JeffCrunch faz em relação a isso. Clique em **Ficheiro > Opções** e, depois, no separador **Segurança**. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

Estas escolhas aplicam-se a todas as aplicações do JeffOffice: se alterar uma numa aplicação, a alteração passa para todas. Uma linha referente a algo que a aplicação não tem indica **Não se aplica** e fica a cinzento.

## Macros

As macros são pequenos programas guardados dentro de um ficheiro. O JeffCrunch mantém-nas no ficheiro, para que continuem lá quando guardar, e **nunca as executa**. Não existe nenhuma definição que as execute.

## Ficheiros ligados e ligações de livros

Isto refere-se a outros livros dos quais um livro obtém números, chamados **ligações**. A escolha define o que o JeffCrunch faz quando abre um livro que tenha algumas.

- **Perguntar primeiro** é a opção habitual. O JeffCrunch pergunta se deve atualizar os números a partir dos outros livros.
- **Atualizar** atualiza-os sem perguntar.
- **Nunca atualizar** mantém os números que o ficheiro guardou.

Uma escolha que faça para um livro em **Dados > Editar Ligações** (**Pedido ao iniciar**) prevalece para esse livro. Um livro que esteja definido como "nunca" é sempre deixado como está. Um livro que esteja noutro computador nunca é lido ao abrir um ficheiro, seja qual for a sua escolha. Pode, contudo, usar **Atualizar Valores** em **Editar Ligações** sempre que quiser.

## Filmes e sons ligados

Isto não se aplica no JeffCrunch: não tem filmes nem sons ligados para reproduzir. A linha fica a cinzento.

## Ficheiros da Internet ou de outros computadores

Um ficheiro que tenha transferido, ou que esteja noutro computador da sua rede, pode não ser seguro. Esta escolha indica como o JeffCrunch o abre.

- **Abrir primeiro como só de leitura** é a opção habitual. O ficheiro abre para poder ler, mas não pode alterá-lo. Uma barra por baixo do ficheiro indica-o. Clique em **Ativar Edição** na barra para alterar apenas esse ficheiro (até fechar o programa). Quando o guardar depois no mesmo local, a marca de transferência é retirada do ficheiro. Em alternativa, use **Guardar Como** para criar uma cópia, ou mude esta definição para a opção seguinte.
- **Abrir normalmente** abre todos os ficheiros da mesma forma.

Um ficheiro que abra com **Abrir Como Cópia**, **Abrir e Reparar** ou **Abrir Só de Leitura** a partir do ecrã **Ficheiro** abre dessa forma, independentemente desta definição.

## Ligações que iniciam um programa

Uma ligação que iniciaria um programa ou um script (um ficheiro com extensão .exe, .bat, .sh, e similares) é **sempre recusada**. O JeffCrunch avisa-o, e nada é executado. Uma ligação para um ficheiro noutro computador nunca é seguida sem que lhe seja pedido primeiro. Nenhuma definição altera isto.

## Consulte também

- [Alterar as definições do JeffCrunch](options)
- [Notas e ligações](notes-links)
