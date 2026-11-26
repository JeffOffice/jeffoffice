# Alterar as definições do JeffCrunch
<!-- keywords: opções de fórmulas, definições de cálculo, alteração máxima, iterações, recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, informações pessoais, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, renomear um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Geral

**Mostrar Mini Barra de Ferramentas ao selecionar** está ativado por predefinição. Decide se a pequena barra de formatação aparece junto ao texto selecionado e no topo do menu de atalhos das células (veja [Escrever nas células](edit-cells)). A escolha é partilhada pelas aplicações JeffOffice.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde vai a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto e apenas um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Só analisa o bloco de células ao lado daquela em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastar da margem da seleção. Desative-o se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero nas células que têm o valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando está desativado, uma célula com 0 parece vazia, mas continua a conter o 0 e soma como 0. A escolha fica guardada com a folha.
- **Colocar uma vírgula decimal em números inteiros introduzidos** chama-se casas decimais automáticas. Se definir **Casas** para 2, ao escrever 1234 introduz 12,34. Se definir para -2, ao escrever 5 introduz 500. Um número escrito com vírgula decimal é introduzido tal como foi escrito.

- **Permitir a edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou a tecla F2 passam para a barra de fórmulas, e a célula só mostra o resultado quando termina a entrada.

## Opções de fórmulas

A página **Fórmulas** define a forma como o livro aberto calcula as suas fórmulas: **Cálculo** (automático, automático exceto tabelas de dados ou manual), se as fórmulas são calculadas antes de o livro ser guardado quando o cálculo é manual, se uma fórmula que se refere a si própria é calculada através de iterações, o número máximo de iterações e a alteração que interrompe as iterações. Estas definições pertencem ao livro, não a si: são guardadas no respetivo ficheiro e não são mantidas para outros livros, e ficam desativadas quando não há nenhum livro aberto. A alteração é mostrada com todos os seus algarismos, e pode introduzi-la como um número, por exemplo 0,0000001 ou 1e-7. Só é gravada uma definição que tenha alterado: **OK** sem nada alterado não acrescenta nenhum passo de Anular e deixa o livro como estava. Cada alteração é um passo de Anular. O formato de ficheiro de livro mais recente e o ficheiro .xls mais antigo mantêm todas as definições. Uma cópia em .ods mantém as definições de iteração, mas não tem onde guardar o cálculo manual; por isso, a cópia calcula as fórmulas à medida que mudam, e o relatório apresentado antes de a cópia ser guardada indica-o. As mesmas definições estão no separador **Fórmulas**.

## Opções de gravação

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações que ainda não guardou, de 1 a 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, feita em segundo plano para que possa continuar a escrever. É eliminada quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que for iniciado. Também pode clicar em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia ficaria desprotegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar ortografia à medida que escreve** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nas outras aplicações; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas na verificação da ortografia, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas passam a aspas curvas, se os travessões, as frações e os ordinais são alterados e se um endereço Web se torna uma ligação. O separador **Símbolos Matemáticos** das Opções de Correção Automática destina-se ao JeffWrite, JeffDeck e JeffPub: o JeffCrunch não transforma uma barra invertida e um nome num símbolo numa célula. Veja [Ortografia](notes-links) e [Escrever nas células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã, e não é guardada no livro.
- **Zoom de um novo livro** é o tamanho com que aparecem as folhas de um novo livro, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de cada coluna desta folha que não tenha largura própria. É o mesmo que **Largura Predefinida** no separador Base.
- **Mostrar os separadores das folhas** e **Mostrar as barras de deslocamento** ocultam ou mostram a linha de separadores das folhas abaixo das células e as barras de deslocamento à direita e em baixo. São escolhas apenas para o seu ecrã, mantidas para todos os livros que abrir.
- **Mostrar a nota de uma célula como** escolhe o aspeto de uma célula com nota: uma pequena marca na célula e a nota quando o ponteiro pousa sobre ela, apenas a marca, ou nada. Uma nota que escolheu mostrar sempre (**Mostrar Nota**) continua na folha em todos os casos.

## Personalizar

O separador **Personalizar** tem três páginas, **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffCrunch. Veja "Escolher as suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os seus grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar, e marque-a para o mostrar novamente. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar com as respetivas teclas de atalho. **Base** não tem caixa, porque está sempre presente, e o mesmo acontece com o botão **Ficheiro**. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, está no fim da lista e não tem caixa.
- **Criar um separador ou grupo próprio.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo seu pode ficar em qualquer separador, incluindo um separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Pode ser colocado no grupo qualquer comando que tenha nome. Um comando só pode estar num grupo uma vez.
- **Renomear.** Escolha um separador, grupo ou comando seu, escreva o novo nome e clique em **Renomear**. Um comando renomeado faz o mesmo que antes, e o novo nome é apenas o texto do seu botão. Para devolver a um comando o seu próprio nome, renomeie-o com um nome vazio.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores do próprio friso, tal como os seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e os seus comandos movem-se da mesma forma dentro do respetivo separador ou grupo. Os grupos do próprio friso mantêm a sua ordem.
- **Remover.** Escolha um separador, grupo ou comando seu e clique em **Remover**.

**Repor Um** repõe o separador escolhido: fica visível, os seus grupos ficam visíveis, e os grupos que lhe acrescentou desaparecem. Um separador seu é removido. **Repor Tudo** repõe o friso inteiro, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram o seu friso, e também todas as janelas que abrir mais tarde. O JeffCrunch guarda apenas o que difere do friso habitual, por isso um separador ou grupo que uma versão posterior acrescente aparece por si. Os seus separadores e grupos recebem Sugestões de Teclas (as letras mostradas ao premir **Alt**) que não coincidem com as restantes. Pode ter 20 separadores próprios, 30 grupos próprios num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** diz respeito à pequena barra de ferramentas no canto superior esquerdo da janela. A lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos** à direita tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a reduzir, do mesmo modo que na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Mover para baixo do friso.** Marque **Mostrar a Barra de Ferramentas Rápida abaixo do friso** para colocar a barra numa linha por baixo do friso, em vez de no canto superior esquerdo. Os dois menus de clique com o botão direito também têm **Mostrar a Barra de Ferramentas Rápida Abaixo do Friso**. O JeffCrunch memoriza esta escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim, e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Repor Um** repõe o comando escolhido: no sítio onde o JeffCrunch normalmente o tem, ou fora da barra, se o JeffCrunch não o costuma ter ali. **Repor Tudo** repõe a barra com que o JeffCrunch vem. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra de ferramentas.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram a sua barra, e também todas as janelas que abrir mais tarde. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem num terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora do ecrã. O JeffCrunch mantém na sua lista um comando que uma versão posterior já não tenha, e volta a mostrá-lo se o comando regressar. Os botões recebem Sugestões de Teclas, os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Opções de privacidade

- **Remover informações pessoais deste ficheiro ao guardar** retira o autor, a pessoa que guardou o ficheiro por último, a empresa, o gestor e os nomes e horas das notas e dos comentários, do ficheiro que é escrito (um livro, uma cópia e um livro enviado por correio eletrónico). O que o ficheiro diz mantém-se. As propriedades personalizadas do ficheiro ficam como estão, e o documento que tem aberto não é alterado. É a mesma escolha em todas as aplicações JeffOffice: altere-a numa e muda em todas. Para o fazer apenas uma vez, use **Inspecionar Livro** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de utilização

**Enviar estatísticas anónimas de utilização** permite partilhar dados sobre a forma como o JeffCrunch é usado. Anónimo significa que os dados não dizem nada sobre quem é. Quando esta opção está ativa, o JeffCrunch envia estes dados uma vez por dia, e no dia em que atualiza: que é o JeffCrunch, a respetiva versão, o seu sistema operativo e idioma, quantas vezes foi iniciado, com que frequência é usado cada comando e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch alguma vez fechar inesperadamente, também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes ou o seu conteúdo, nem nada que tenha escrito. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha pertence a todas as aplicações JeffOffice: altere-a aqui e muda em cada uma delas. O JeffCrunch pergunta sobre o assunto na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** aparece por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para eliminar as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Segurança

O separador **Segurança** decide o que o JeffCrunch faz em relação aos elementos para os quais um ficheiro pode apontar: ficheiros ligados, filmes e sons ligados e ficheiros da Internet. As macros ficam guardadas num ficheiro e nunca são executadas, e uma ligação que iniciaria um programa é sempre recusada. Veja [Segurança](security).

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
