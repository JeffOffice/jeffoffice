# Alterar as definições do JeffCrunch
<!-- keywords: recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, renomear um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde segue a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático de Texto. Quando escreve as primeiras letras de um texto, e só um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Só analisa o bloco de células ao lado daquela em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastamento do limite da seleção. Desative-a se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero em células com valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando está desativado, uma célula com o valor 0 parece vazia, mas continua a conter o 0 e é somada como 0. A escolha fica guardada com a folha.
- **Colocar um ponto decimal em números inteiros escritos** chama-se casas decimais automáticas. Defina **Casas** como 2, e ao escrever 1234 é introduzido 12,34. Defina-o como -2, e ao escrever 5 é introduzido 500. Um número escrito com um ponto decimal é introduzido tal como foi escrito.

- **Permitir edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou F2 vai para a barra de fórmulas, e a célula só mostra o resultado quando termina a entrada.

## Opções de gravação

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações que ainda não guardou, de 1 a 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, criada em segundo plano para que possa continuar a escrever. É removida quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que for iniciado. Também pode clicar em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia não ficaria protegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar ortografia ao escrever** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nas outras aplicações; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas na verificação ortográfica, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas se transformam em aspas curvas, se travessões, frações e ordinais são alterados, e se um endereço Web se transforma numa ligação. Consulte [Ortografia](notes-links) e [Escrever nas células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã, não guardada no livro.
- **Zoom de um novo livro** define a dimensão com que as folhas de um novo livro são apresentadas, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de cada coluna desta folha que não tenha largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Personalizar

O separador **Personalizar** tem três páginas, **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffCrunch. Consulte "Escolher as suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os respetivos grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar e marque-a para o voltar a mostrar. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar pelas teclas de atalho. **Base** não tem caixa, porque fica sempre visível, e o mesmo acontece com o botão **Ficheiro**. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar um separador ou grupo próprio.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer separador, incluindo um separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para restringir a lista) e clique em **Adicionar Comando**. Pode incluir qualquer comando que tenha um nome. Um comando só pode estar num grupo uma vez.
- **Renomear.** Escolha um separador, grupo ou comando próprio, escreva o novo nome e clique em **Renomear**. Um comando renomeado faz o mesmo que antes, e o novo nome é apenas o texto do seu botão. Renomeie um comando com um nome vazio para lhe devolver o nome próprio.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores do próprio friso, além dos seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e comandos movem-se da mesma forma dentro do respetivo separador ou grupo. Os grupos do próprio friso mantêm a ordem.
- **Remover.** Escolha um separador, grupo ou comando próprio e clique em **Remover**.

**Repor Um** repõe o separador escolhido: fica visível, os seus grupos ficam visíveis e os grupos que lhe acrescentou são eliminados. Um separador próprio é removido. **Repor Tudo** repõe o friso inteiro, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram o seu friso, e também todas as janelas que abrir mais tarde. O JeffCrunch guarda apenas o que difere do friso habitual, por isso um separador ou grupo que uma versão posterior acrescente aparece por si só. Os seus separadores e grupos recebem Sugestões de Teclas (as letras que aparecem ao premir **Alt**) que não coincidem com as restantes. Pode ter 20 separadores próprios, 30 grupos próprios num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** diz respeito à pequena barra de ferramentas no canto superior esquerdo da janela. A lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos** à direita tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a restringir, como na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Adicionar um separador.** Clique em **Adicionar Separador** para pôr uma linha fina no fim e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Repor Um** repõe o comando escolhido: no sítio onde o JeffCrunch normalmente o coloca, ou fora da barra quando o JeffCrunch não o costuma pôr lá. **Repor Tudo** repõe a barra com que o JeffCrunch é fornecido. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffCrunch mostram a sua barra, e também todas as janelas que abrir mais tarde. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca fique fora do ecrã. O JeffCrunch mantém um comando na sua lista quando uma versão posterior já não o tem, e volta a mostrá-lo se o comando regressar. Os botões recebem Sugestões de Teclas, os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Opções de privacidade

- **Remover informações pessoais deste ficheiro ao guardar** retira o autor, a pessoa que guardou o ficheiro por último, a empresa, o gestor, e os nomes e horas das notas e comentários, do ficheiro que é gravado (um livro, uma cópia e um livro enviado por correio eletrónico). O conteúdo do ficheiro mantém-se igual. As propriedades personalizadas do ficheiro são mantidas, e o documento que tem aberto não é alterado. É a mesma opção em todas as aplicações JeffOffice: altere-a numa e muda em todas. Para fazer isto apenas uma vez, use **Inspecionar Livro** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de utilização

**Enviar estatísticas anónimas de utilização** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não revelam quem é. Quando esta opção está ativada, o JeffCrunch envia estes dados uma vez por dia, e no dia em que atualiza: que se trata do JeffCrunch, a sua versão, o seu sistema operativo e idioma, quantas vezes foi iniciado, com que frequência cada comando é usado, e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes, nem qualquer conteúdo, nem o que escreve. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações JeffOffice: altere-a aqui e muda em cada uma delas. O JeffCrunch pergunta sobre isto na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, junto à caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Aí se indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para eliminar as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contado nem enviado, e o JeffCrunch não faz a pergunta da primeira utilização.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
