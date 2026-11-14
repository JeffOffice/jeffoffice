# Alterar as definições do JeffCrunch

<!-- keywords: recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, renomear um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde vai a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir o restante de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático de texto. Quando escreve as primeiras letras de um texto, e apenas um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Este recurso analisa apenas o bloco de células ao lado da célula em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastar da margem da seleção. Desative esta opção se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar zero em células com valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando estão desativados, uma célula que contém 0 parece vazia, mas continua a conter o 0 e é somada como 0. A escolha é guardada com a folha.
- **Colocar um separador decimal em números inteiros escritos** chama-se casas decimais automáticas. Defina **Casas** como 2, e se escrever 1234 será introduzido 12,34. Se definir -2, e escrever 5 será introduzido 500. Um número escrito com separador decimal é introduzido tal como o escreveu.

- **Permitir edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou a tecla F2 segue para a barra de fórmulas, e a célula só mostra o resultado quando termina.

## Opções de gravação

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações ainda não guardadas, de 1 a 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, criada em segundo plano para que possa continuar a escrever. É eliminada quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que for iniciado. Também pode clicar em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia não ficaria protegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar ortografia ao escrever** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nas outras aplicações; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas ao verificar a ortografia, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas passam a aspas curvas, se os travessões, as frações e os ordinais são alterados, e se um endereço Web se transforma numa ligação. Consulte [Ortografia](notes-links) e [Escrever em células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã, não guardada no livro.
- **Zoom de um novo livro** define a dimensão com que as folhas de um novo livro são apresentadas, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de todas as colunas desta folha que não tenham largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Personalizar

O separador **Personalizar** tem três páginas, **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffCrunch. Consulte "Escolher as suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os respetivos grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar, e marque-a para o voltar a mostrar. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar com as teclas de atalho. **Base** não tem caixa, porque está sempre visível, e o mesmo acontece com o botão **Ficheiro**. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar um separador ou grupo próprio.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo próprio pode ir em qualquer separador, incluindo um separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para restringir a lista) e clique em **Adicionar Comando**. Pode ser incluído qualquer comando que tenha nome. Um comando só pode estar num grupo uma vez.
- **Renomear.** Escolha um separador, grupo ou comando próprio, escreva o novo nome e clique em **Renomear**. Um comando que renomeia continua a fazer o mesmo, e o novo nome é apenas o que o respetivo botão mostra. Para renomear um comando com um nome vazio e lhe devolver o nome próprio.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores do próprio friso, bem como os seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e comandos movem-se da mesma forma dentro do respetivo separador ou grupo. Os grupos do próprio friso mantêm a ordem.
- **Retirar.** Escolha um separador, grupo ou comando próprio e clique em **Remover**.

**Repor Um** repõe o separador escolhido: fica visível, os seus grupos ficam visíveis, e os grupos que lhe acrescentou desaparecem. Um separador próprio é retirado. **Repor Tudo** repõe o friso inteiro, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram o seu friso, e também todas as janelas que abrir mais tarde. O JeffCrunch guarda apenas o que difere do friso habitual, por isso um separador ou grupo que uma versão posterior acrescente aparece por si. Os seus separadores e grupos recebem Sugestões de Teclas (as letras mostradas quando prime **Alt**) que não coincidem com as restantes. Pode ter 20 separadores próprios, 30 grupos próprios num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** diz respeito à pequena barra de ferramentas no canto superior esquerdo da janela. A lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos** da direita tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a restringir, tal como na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Mover para baixo do friso.** Marque **Mostrar a Barra de Ferramentas Rápida abaixo do friso** para colocar a barra numa linha por baixo do friso, em vez de no canto superior esquerdo. Os dois menus do botão direito também têm **Mostrar a Barra de Ferramentas Rápida Abaixo do Friso**. O JeffCrunch memoriza a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e depois mova-a para onde pretender.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Retirar.** Escolha-o e clique em **Remover**.

**Repor Um** repõe o comando escolhido: no sítio onde o JeffCrunch normalmente o tem, ou fora da barra quando o JeffCrunch não o costuma ter lá. **Repor Tudo** repõe a barra de ferramentas com que o JeffCrunch vem. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra de ferramentas.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram a sua barra, e também todas as janelas que abrir mais tarde. A barra pode ter, no total, 100 comandos e separadores. Quando há mais botões do que cabem num terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora do ecrã. O JeffCrunch mantém na sua lista um comando que uma versão posterior já não tenha, e volta a mostrá-lo se o comando regressar. Os botões recebem Sugestões de Teclas: os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Opções de privacidade

- **Remover informações pessoais deste ficheiro ao guardar** retira o autor, a pessoa que guardou o ficheiro pela última vez, a empresa, o gestor, bem como os nomes e as horas das notas e dos comentários, do ficheiro que é gravado (um livro, uma cópia e um livro enviado por correio eletrónico). O conteúdo do ficheiro mantém-se como está. As propriedades personalizadas do ficheiro são deixadas como estão, e o documento que tem aberto não é alterado. Esta escolha é igual em todas as aplicações JeffOffice: altere-a numa e muda em todas. Para fazer isto apenas uma vez, use **Inspecionar Livro** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de utilização

**Enviar estatísticas anónimas de utilização** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não dizem nada sobre quem é. Quando esta opção está ativada, o JeffCrunch envia estes dados uma vez por dia, e no dia em que atualiza: que se trata do JeffCrunch, a sua versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência cada comando é usado, e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch alguma vez fechar inesperadamente, também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes, nem qualquer conteúdo, nem o que escreveu. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações JeffOffice: altere-a aqui e muda em cada uma delas. O JeffCrunch pergunta sobre isto na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Nele se explica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para eliminar as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Ao desativá-las, o número é apagado do seu computador, e sem ele não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
