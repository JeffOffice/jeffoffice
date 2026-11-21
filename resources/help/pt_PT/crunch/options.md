# Alterar as definições do JeffCrunch
<!-- keywords: opções de fórmulas, definições de cálculo, alteração máxima, iteração, recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, renomear um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Geral

**Mostrar Mini Barra de Ferramentas ao selecionar** está ativado por predefinição. Decide se a pequena barra de formatação aparece junto ao texto selecionado e no topo do menu de atalho das células (veja [Escrever em células](edit-cells)). A escolha é partilhada pelas aplicações JeffOffice.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde vai a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto, e só um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Só analisa o bloco de células ao lado daquela em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastamento do limite da seleção. Desative-a se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero nas células que têm o valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando está desativado, uma célula com 0 parece vazia, mas continua a conter o 0 e soma como 0. A escolha é guardada com a folha.
- **Colocar uma vírgula decimal em números inteiros escritos** chama-se casas decimais automáticas. Defina **Casas** para 2, e ao escrever 1234 introduz 12,34. Defina-o para -2, e ao escrever 5 introduz 500. Um número escrito com vírgula decimal é introduzido tal como o escreveu.

- **Permitir edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou F2 vai para a barra de fórmulas, e a célula só mostra o resultado quando termina.

## Opções de fórmulas

A página **Fórmulas** define a forma como o livro aberto calcula as suas fórmulas: **Cálculo** (automático, automático exceto tabelas de dados, ou manual), se as fórmulas são calculadas antes de o livro ser guardado quando o cálculo é manual, se uma fórmula que se refere a si própria é calculada através de iterações, o número máximo de iterações e a alteração que faz parar as iterações. Estas definições pertencem ao livro, não a si: são guardadas no respetivo ficheiro e não são mantidas para outros livros, e as escolhas ficam desativadas quando nenhum livro está aberto. A alteração é mostrada com todos os seus algarismos, e pode escrevê-la como um número, como 0,0000001 ou 1e-7. Só é gravada uma definição que altere: **OK** sem nada alterado não acrescenta nenhum passo de Anular e deixa o livro como estava. Uma alteração é um passo de Anular. O ficheiro de livro mais recente e o ficheiro .xls mais antigo mantêm todas as definições. Uma cópia .ods mantém as definições de iteração, mas não tem onde guardar o cálculo manual, por isso a cópia calcula as fórmulas à medida que mudam, e o relatório antes de a cópia ser guardada indica-o. As mesmas definições estão no separador **Fórmulas**.

## Opções de gravação

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações que não guardou, entre 1 e 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, criada em segundo plano para que possa continuar a escrever. É removida quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que arrancar. Também pode clicar em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia não ficaria protegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar ortografia ao escrever** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nas outras aplicações; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas na verificação ortográfica, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas passam a aspas curvas, se os travessões, as frações e os ordinais são alterados, e se um endereço Web passa a ser uma ligação. Veja [Ortografia](notes-links) e [Escrever em células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã, não guardada no livro.
- **Zoom de um novo livro** define a dimensão das folhas de um novo livro, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de todas as colunas desta folha que não têm uma largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Personalizar

O separador **Personalizar** tem três páginas, **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffCrunch. Veja "Escolher as suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os respetivos grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar, e marque-a para o mostrar de novo. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar com as respetivas teclas de atalho. **Base** não tem caixa, porque está sempre presente, e o botão **Ficheiro** também não. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, está no fim da lista e não tem caixa.
- **Criar um separador ou grupo próprio.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo próprio pode ir em qualquer separador, incluindo um separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Pode ser incluído qualquer comando que tenha um nome. Um comando só pode estar num grupo uma vez.
- **Renomear.** Escolha um separador, grupo ou comando próprio, escreva o novo nome e clique em **Mudar o Nome**. Um comando que renomeia faz o mesmo que antes, e o novo nome é apenas o que o seu botão mostra. Para renomear um comando com um nome vazio e lhe devolver o nome original, deixe o nome vazio e clique em **Mudar o Nome**.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores do próprio friso, bem como os seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e comandos movem-se da mesma forma dentro do respetivo separador ou grupo. Os grupos do próprio friso mantêm a sua ordem.
- **Retirar.** Escolha um separador, grupo ou comando próprio e clique em **Remover**.

**Repor Um** devolve o separador escolhido ao estado original: fica visível, os seus grupos ficam visíveis, e os grupos que lhe acrescentou desaparecem. Um separador próprio é removido. **Repor Tudo** devolve todo o friso ao estado original, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram o seu friso, e também todas as janelas que abrir mais tarde. O JeffCrunch guarda apenas o que difere do friso habitual, por isso um separador ou grupo que uma versão posterior acrescente aparece por si. Os seus separadores e grupos têm Sugestões de Teclas (as letras que aparecem quando prime **Alt**) que não entram em conflito com as outras. Pode ter 20 separadores próprios, 30 grupos próprios num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** diz respeito à pequena barra no canto superior esquerdo da janela. A lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos** à direita tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a reduzir, do mesmo modo que na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra. Um comando que abre um menu mantém o seu menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Mover para baixo do friso.** Marque **Mostrar a Barra de Ferramentas Rápida abaixo do friso** para colocar a barra numa linha por baixo do friso, em vez de no canto superior esquerdo. Os dois menus de clique com o botão direito também têm **Mostrar a Barra de Ferramentas Rápida Abaixo do Friso**. O JeffCrunch memoriza a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim, e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Retirar.** Escolha-o e clique em **Remover**.

**Repor Um** devolve o comando escolhido ao sítio onde o JeffCrunch normalmente o tem, ou retira-o da barra quando o JeffCrunch normalmente não o tem ali. **Repor Tudo** devolve a barra com que o JeffCrunch vem. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra de ferramentas.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram a sua barra, e também todas as janelas que abrir mais tarde. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem num terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora do ecrã. O JeffCrunch mantém na sua lista um comando que uma versão posterior já não tenha, e volta a mostrá-lo se o comando regressar. Os botões têm Sugestões de Teclas, os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Opções de privacidade

- **Remover informações pessoais deste ficheiro ao guardar** retira o autor, a pessoa que guardou o ficheiro por último, a empresa, o gestor, e os nomes e horas das notas e comentários, do ficheiro que é escrito (um livro, uma cópia, e um livro enviado por correio eletrónico). O que o ficheiro diz mantém-se como está. As propriedades personalizadas do ficheiro são mantidas, e o documento que tem aberto não é alterado. A escolha é a mesma em todas as aplicações JeffOffice: altere-a numa e muda em todas. Para fazer isto apenas uma vez, use **Inspecionar Livro** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de utilização

**Enviar estatísticas anónimas de utilização** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não dizem nada sobre quem é. Quando esta opção está ativada, o JeffCrunch envia estes dados uma vez por dia, e no dia em que atualiza: que é o JeffCrunch, a respetiva versão, o seu sistema operativo e idioma, quantas vezes foi iniciado, a frequência com que cada comando é usado, e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch fechar inesperadamente, também envia um relatório de falha na próxima vez que arrancar. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes, nem nada do que contêm, nem nada do que escreve. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha pertence a todas as aplicações JeffOffice: altere-a aqui e muda em cada uma delas. O JeffCrunch pergunta sobre isto na primeira vez que arranca, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é mostrado abaixo da caixa de seleção. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, mostra **nenhum**.
- Para que as estatísticas que já temos sejam eliminadas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador, e sem ele não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira utilização.

## Segurança

O separador **Segurança** decide o que o JeffCrunch faz em relação aos elementos para onde um ficheiro pode apontar: ficheiros ligados, filmes e sons ligados, e ficheiros da Internet. As macros são guardadas num ficheiro e nunca são executadas, e uma ligação que iniciaria um programa é sempre recusada. Veja [Segurança](security).

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
