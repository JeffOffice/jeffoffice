# Alterar as definições do JeffCrunch

<!-- keywords: recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, renomear um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Após premir Enter, mover a seleção** indica para onde segue a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir o fim de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto, e só um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Só analisa o bloco de células ao lado daquela em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastamento da margem da seleção. Desative-o se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero em células com valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando está desativado, uma célula com 0 parece vazia, mas continua a conter o 0 e é somada como 0. A escolha é guardada com a folha.
- **Colocar um separador decimal em números inteiros introduzidos** chama-se casas decimais automáticas. Se definir **Casas** como 2, ao escrever 1234 introduz 12,34. Se o definir como -2, ao escrever 5 introduz 500. Um número que escreva com separador decimal é introduzido tal como o escreveu.

- **Permitir edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou a tecla F2 segue para a barra de fórmulas, e a célula só mostra o resultado quando termina.

## Opções de gravação

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações que ainda não guardou, de 1 a 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, criada em segundo plano para que possa continuar a escrever. É removida quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que for iniciado. Também pode clicar em **Ficheiro > Abrir > Recuperar trabalho não guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia não ficaria protegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar ortografia à medida que escreve** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nas outras aplicações; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas na verificação ortográfica, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas se tornam curvas, se os travessões, as frações e os ordinais são alterados, e se um endereço Web se torna uma hiperligação. Consulte [Ortografia](notes-links) e [Escrever em células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no seu ecrã. É uma escolha apenas para o seu ecrã, não guardada no livro.
- **Zoom de um novo livro** é o tamanho com que as folhas de um novo livro são apresentadas, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de cada coluna desta folha que não tem largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Personalizar

O separador **Personalizar** tem três páginas, **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffCrunch. Consulte "Escolher as suas próprias teclas de atalho" no tópico sobre o teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os seus grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar e marque-a para o voltar a mostrar. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar com as teclas de atalho. **Base** não tem caixa, porque está sempre presente, e o mesmo acontece com o botão **Ficheiro**. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar o seu próprio separador ou grupo.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo seu pode ficar em qualquer separador, incluindo um separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Pode ser incluído qualquer comando que tenha um nome. Um comando só pode estar num grupo uma vez.
- **Renomear.** Escolha um separador, grupo ou comando seu, escreva o novo nome e clique em **Renomear**. Um comando que renomeia continua a fazer o mesmo, e o novo nome é apenas o que o seu botão mostra. Para renomear um comando com um nome vazio e lhe devolver o nome próprio.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores do próprio friso, bem como os seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e comandos movem-se da mesma forma dentro do seu separador ou grupo. Os grupos do próprio friso mantêm a ordem.
- **Remover.** Escolha um separador, grupo ou comando seu e clique em **Remover**.

**Repor Uma** repõe o separador escolhido: fica visível, os seus grupos ficam visíveis e os grupos que lhe acrescentou desaparecem. Um separador seu é removido. **Repor Tudo** repõe o friso inteiro, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram o seu friso, e também o mostram todas as janelas que abrir mais tarde. O JeffCrunch guarda apenas o que difere do friso habitual, por isso um separador ou grupo que uma versão posterior acrescente aparece por si só. Os seus separadores e grupos recebem Sugestões de Teclas (as letras que aparecem ao premir **Alt**) que não entram em conflito com as restantes. Pode ter 20 separadores seus, 30 grupos seus num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** é para a pequena barra de ferramentas no canto superior esquerdo da janela. A sua lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos** à direita tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a reduzir, do mesmo modo que na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra de ferramentas. Um comando que abre um menu mantém o seu menu na barra. Um comando só pode estar na barra de ferramentas uma vez.
- **Clique com o botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Mover para baixo do friso.** Marque **Mostrar a Barra de Ferramentas Rápida abaixo do friso** para colocar a barra numa linha por baixo do friso, em vez de no canto superior esquerdo. Os dois menus de clique com o botão direito também têm **Mostrar a Barra de Ferramentas Rápida Abaixo do Friso**. O JeffCrunch memoriza a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e, em seguida, mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Repor Uma** repõe o comando escolhido: no sítio onde o JeffCrunch o costuma ter ou fora da barra de ferramentas, quando o JeffCrunch não o costuma ter ali. **Repor Tudo** repõe a barra de ferramentas com que o JeffCrunch é fornecido. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra de ferramentas.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram a sua barra de ferramentas, e também a mostram todas as janelas que abrir mais tarde. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora do ecrã. O JeffCrunch mantém um comando na sua lista quando uma versão posterior o deixa de ter, e volta a mostrá-lo se o comando regressar. Os botões recebem Sugestões de Teclas, os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Opções de privacidade

- **Remover informações pessoais deste ficheiro ao guardar** retira o autor, a pessoa que guardou o ficheiro por último, a empresa, o gestor, e os nomes e horas das notas e comentários, do ficheiro que é escrito (um livro, uma cópia e um livro enviado por correio eletrónico). O conteúdo do ficheiro mantém-se como está. As propriedades personalizadas do ficheiro são deixadas como estão, e o documento que tem aberto não é alterado. É a mesma escolha em todas as aplicações JeffOffice: altere-a numa e muda em todas. Para o fazer apenas uma vez, use **Inspecionar Livro** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de utilização

**Enviar estatísticas anónimas de utilização** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não dizem nada sobre quem é. Quando esta opção está ativada, o JeffCrunch envia estes dados uma vez por dia, e no dia em que atualiza: que se trata do JeffCrunch, a sua versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência cada comando é usado, e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch fechar alguma vez inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes ou qualquer conteúdo, nem aquilo que escreve. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha pertence a todas as aplicações JeffOffice: altere-a aqui e muda em cada uma delas. O JeffCrunch pergunta sobre ela na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade** ao lado da caixa de verificação abre o nosso aviso de privacidade em jeffoffice.com/privacy. Nele se indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para que as estatísticas que já temos sejam eliminadas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber que registos são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
