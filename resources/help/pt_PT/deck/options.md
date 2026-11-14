# Alterar as definições do JeffDeck

<!-- keywords: opções, definições, preferências, tema, modo escuro, idioma, nome de utilizador, iniciais, régua, ajustar, ortografia, correção automática, aspas, atualizações, versão beta, estatísticas de utilização, acerca de, guardar, incorporar tipos de letra, tipos de letra, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, mudar o nome de um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffDeck**. Clique em **Ficheiro > Opções** para a abrir. Tem cinco separadores. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Geral

- **Nome de utilizador** e **Iniciais** são apresentados nos comentários que escreve. Veja [Comentários](comments).
- **Tema** pode ser **Utilizar definição do sistema**, **Claro** ou **Escuro**. A alteração é apresentada de imediato.
- **Idioma de apresentação** é o idioma das palavras da própria interface do JeffDeck. **Utilizar idioma do sistema** segue o idioma do computador. Os outros idiomas aparecem nesta lista quando existirem traduções disponíveis. Um novo idioma só é aplicado na próxima vez que abrir o JeffDeck.
- **Verificar atualizações quando o JeffDeck é iniciado** faz com que o JeffDeck procure uma versão mais recente sempre que é iniciado. Se existir, a janela **Atualização Disponível** mostra as novidades. Escolha **Atualizar Agora**, **Mais Tarde** ou **Ignorar Esta Versão**. No Windows, **Atualizar Agora** transfere o programa de instalação. Este instala a nova versão de todas as aplicações JeffOffice e pede que feche as que estiverem abertas. Nos outros computadores, o botão é **Abrir Página de Transferência**. Abre a página onde pode obter a nova versão. Se o JeffPub também estiver aberto, apenas um dos dois o avisa de uma nova versão por dia.
- **Obter versões beta (compilações antecipadas para testes)** faz com que o JeffDeck também ofereça versões beta. Estas saem antes das versões normais e podem ter imperfeições.
- **Enviar estatísticas anónimas de utilização** permite partilhar dados sobre a forma como o JeffDeck é utilizado. Anónimo significa que os dados não dizem nada sobre si. Quando esta opção está ativada, o JeffDeck envia estes dados uma vez por dia e no dia em que atualizar: que se trata do JeffDeck, a sua versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência é utilizado cada comando e que tipo de ficheiro abriu ou guardou (por exemplo, .pptx). Se o JeffDeck fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O relatório indica em que ponto do programa parou, para que o problema possa ser encontrado e corrigido. O JeffDeck nunca envia os seus ficheiros, os respetivos nomes ou qualquer conteúdo deles, nem o que tiver escrito. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha. O JeffDeck pergunta sobre isto na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffDeck**. Não pergunta se já tiver escolhido no JeffPub ou na instalação.
- **Aviso de privacidade**, junto a essa caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Nele indicamos o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para eliminar as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffDeck, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Assim, nada é contabilizado nem enviado, e o JeffDeck não faz a pergunta da primeira utilização.

**Nome de utilizador**, **Iniciais**, **Tema** e **Idioma de apresentação**, bem como as opções de atualizações e de estatísticas, são comuns a todas as aplicações JeffOffice: se alterar uma delas aqui, a alteração é aplicada em todas.

## Verificação

Estas caixas de verificação controlam a ortografia e as ajudas à escrita. Veja [Escrever e formatar texto](text).

- **Verificar a ortografia ao escrever**
- **Verificar a gramática à medida que escreve** assinala com uma linha azul pontilhada os erros claros de gramática e de estilo em texto em inglês, francês, alemão e espanhol.
- **Definições de gramática** abre uma lista dos tipos de problemas a procurar: gramática, pontuação, maiúsculas, palavras confundidas, repetições e estilo (que fica desativado até o ativar).
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao escrever (Correção Automática)** corrige erros comuns de escrita.
- **Substituir aspas retas por aspas inteligentes** transforma " e ' em aspas curvas.
- **Formatar automaticamente ao escrever (travessões, frações, ordinais, listas automáticas)**
- **Transformar endereços Web em hiperligações à medida que escreve** faz de um endereço Web ou de correio eletrónico uma hiperligação quando premir a barra de espaço depois dele. Anular retira apenas a hiperligação.

## Ver

- **Mostrar a régua** mostra as réguas ao longo das margens do diapositivo. É o mesmo que **Ver > Mostrar > Régua**, e aplica-se a todas as janelas, atuais e às que abrir depois.
- **Ajustar objetos a outros objetos** faz com que um objeto que arrastar fique alinhado com os restantes objetos do diapositivo.

## Guardar

- **Incorporar tipos de letra no ficheiro** coloca dentro do ficheiro, ao guardar, os tipos de letra usados nos diapositivos, para que estes tenham o mesmo aspeto num computador que não os tenha. Fica ativado até desmarcar. Inclui apenas os tipos de letra que vêm com o JeffDeck e aumenta o tamanho do ficheiro. O tópico Abrir e guardar apresentações tem os detalhes, na secção "Tipos de letra no ficheiro".
- **Remover informações pessoais deste ficheiro ao guardar** retira do ficheiro gravado (um ficheiro .pptx ou .odp, um modelo e uma apresentação enviada por correio eletrónico) o autor, a pessoa que guardou o ficheiro por último, a empresa, o gestor e os nomes, as iniciais e as horas dos comentários. O conteúdo do ficheiro mantém-se. As propriedades personalizadas do ficheiro são deixadas como estão, e o documento que tem aberto não é alterado. É a mesma opção em todas as aplicações JeffOffice: se a alterar numa, altera-se em todas. A mesma caixa está na página **Informações**.

## Personalizar

O separador **Personalizar** tem três páginas: **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffDeck. Veja "Escolher as suas próprias teclas de atalho" no tópico sobre o teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os seus grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar e marque-a para o voltar a mostrar. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar com as respetivas teclas de atalho. **Base** não tem caixa, porque está sempre visível, e o mesmo acontece com o botão **Ficheiro**. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar um separador ou grupo próprio.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer separador, incluindo num separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Pode ser incluído qualquer comando que tenha um nome. Um comando só pode estar uma vez num grupo.
- **Mudar o nome.** Escolha um separador, grupo ou comando próprio, escreva o novo nome e clique em **Renomear**. Um comando ao qual mudou o nome faz o mesmo que antes, e o novo nome é apenas o texto do seu botão. Para mudar o nome de um comando para o seu nome original, mude-o para um nome vazio.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores próprios do friso, bem como os seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e comandos movem-se da mesma forma dentro do respetivo separador ou grupo. Os grupos do próprio friso mantêm a sua ordem.
- **Remover.** Escolha um separador, grupo ou comando próprio e clique em **Remover**.

**Repor Um** repõe o separador escolhido: fica visível, os seus grupos ficam visíveis, e os grupos que lhe acrescentou são eliminados. Um separador próprio é removido. **Repor Tudo** repõe o friso inteiro, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffDeck mostram o seu friso, e também todas as janelas que abrir mais tarde. O JeffDeck guarda apenas o que difere do friso habitual, pelo que um separador ou grupo que uma versão posterior acrescente aparece por si. Os seus separadores e grupos recebem Sugestões de Teclas (as letras que aparecem ao premir **Alt**) que não colidem com as restantes. Pode ter 20 separadores próprios, 30 grupos próprios num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** serve para a pequena barra no canto superior esquerdo da janela. A lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos** da direita tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a reduzir, tal como na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar uma vez na barra.
- **Botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. No mesmo menu estão **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Mostrar abaixo do friso.** Marque **Mostrar a Barra de Ferramentas Rápida abaixo do friso** para colocar a barra numa linha por baixo do friso, em vez de a colocar no canto superior esquerdo. Os dois menus do botão direito têm também **Mostrar a Barra de Ferramentas Rápida Abaixo do Friso**. O JeffDeck memoriza a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para pôr uma linha fina no fim e, depois, mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Repor Um** repõe o comando escolhido: no sítio onde o JeffDeck normalmente o tem ou fora da barra, se o JeffDeck não o tiver lá normalmente. **Repor Tudo** repõe a barra com que o JeffDeck vem. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra.

Nada muda até clicar em **OK**. Depois, todas as janelas abertas do JeffDeck mostram a sua barra, e também todas as janelas que abrir mais tarde. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem num terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora do ecrã. O JeffDeck mantém na sua lista um comando que uma versão posterior já não tenha, e volta a mostrá-lo se o comando regressar. Os botões recebem Sugestões de Teclas, os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Relacionados

- [Separador Ver](tab-view)
- [Recuperar trabalho não guardado](recover)
- [O ecrã Ficheiro](file-screen)
