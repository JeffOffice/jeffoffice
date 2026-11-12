# Alterar as definições do JeffDeck

<!-- keywords: opções, definições, preferências, tema, modo escuro, idioma, nome de utilizador, iniciais, régua, ajustar, ortografia, correção automática, aspas, atualizações, beta, estatísticas de utilização, acerca de, guardar, incorporar tipos de letra, tipos de letra, personalizar teclado, personalizar o friso, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar um comando à barra de ferramentas rápida, ocultar um separador, ocultar um grupo, adicionar um separador, adicionar um grupo, renomear um separador, repor o friso, alterar teclas de atalho, reatribuir teclas, repor teclas -->

As suas definições estão na janela **Opções do JeffDeck**. Clique em **Ficheiro > Opções** para a abrir. Tem cinco separadores. Clique em **OK** para manter as alterações, ou em **Cancelar** para as descartar.

## Geral

- **Nome de utilizador** e **Iniciais** são apresentados nos comentários que escreve. Veja [Comentários](comments).
- **Tema** é **Utilizar definição do sistema**, **Claro** ou **Escuro**. A alteração é apresentada de imediato.
- **Idioma de apresentação** é o idioma das palavras do próprio JeffDeck. **Utilizar idioma do sistema** segue o idioma do computador. Os outros idiomas aparecem nesta lista quando existem traduções disponíveis. Um novo idioma só entra em vigor da próxima vez que abrir o JeffDeck.
- **Procurar atualizações quando o JeffDeck é iniciado** faz com que o JeffDeck procure uma versão mais recente sempre que é iniciado. Se existir, a janela **Atualização Disponível** mostra as novidades. Escolha **Atualizar Agora**, **Mais Tarde** ou **Ignorar Esta Versão**. No Windows, **Atualizar Agora** transfere o programa de instalação. Instala a nova versão de todas as aplicações JeffOffice e pede-lhe que feche as que estiverem abertas. Noutros computadores, o botão chama-se **Abrir Página de Transferência**. Abre a página onde pode obter a nova versão. Se o JeffPub também estiver aberto, apenas um dos dois o avisa de uma nova versão por dia.
- **Obter versões beta (compilações antecipadas para testes)** faz com que o JeffDeck ofereça também as versões beta. Estas saem antes das versões normais. Podem ter imperfeições.
- **Enviar estatísticas de utilização anónimas** permite partilhar dados sobre a forma como o JeffDeck é utilizado. Anónimo significa que os dados não revelam quem é. Com esta opção ativada, o JeffDeck envia estes dados uma vez por dia, e no dia em que atualizar: que se trata do JeffDeck, a sua versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência é usado cada comando, e que tipo de ficheiro abriu ou guardou (por exemplo, .pptx). Se o JeffDeck fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O relatório indica em que ponto do programa parou, para que o problema possa ser encontrado e corrigido. O JeffDeck nunca envia os seus ficheiros, os respetivos nomes, nem qualquer conteúdo, nem nada do que escreveu. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha. O JeffDeck pergunta sobre isto a primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffDeck**. Não pergunta se já tiver escolhido no JeffPub ou na instalação.
- **Aviso de privacidade**, junto a esta caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não revela nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para que as estatísticas que já temos sejam eliminadas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Ao desativá-las, o número é apagado do seu computador e, sem ele, não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffDeck, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Assim, nada é contabilizado nem enviado, e o JeffDeck não faz a pergunta da primeira utilização.

**Nome de utilizador**, **Iniciais**, **Tema** e **Idioma de apresentação**, bem como as opções de atualizações e estatísticas, pertencem a todas as aplicações JeffOffice: se alterar uma delas aqui, a alteração aplica-se em todas.

## Verificação

Estas caixas de verificação controlam a ortografia e as ajudas à escrita. Veja [Escrever e formatar texto](text).

- **Verificar a ortografia ao escrever**
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao escrever (Correção Automática)** corrige erros comuns de escrita.
- **Substituir aspas retas por aspas inteligentes** transforma " e ' em aspas curvas.
- **Formatação automática ao escrever (travessões, frações, ordinais, listas automáticas)**
- **Transformar endereços Web em hiperligações à medida que escreve** transforma um endereço Web ou de correio eletrónico numa hiperligação quando prime a barra de espaço depois dele. Anular desfaz apenas a hiperligação.

## Ver

- **Mostrar réguas** apresenta as réguas ao longo das margens do diapositivo. É o mesmo que **Ver > Mostrar > Régua**, e aplica-se a todas as janelas, atuais e às que abrir mais tarde.
- **Ajustar objetos a outros objetos** faz com que um objeto que arraste se alinhe com os outros objetos do diapositivo.

## Guardar

- **Incorporar tipos de letra no ficheiro** coloca dentro do ficheiro, ao guardar, os tipos de letra usados nos diapositivos, para que tenham o mesmo aspeto num computador que não tenha esses tipos de letra. Esta opção fica ativada até a desmarcar. Inclui apenas os tipos de letra que vêm com o JeffDeck, e o ficheiro fica maior. O tópico Abrir e guardar apresentações tem os detalhes, na secção "Tipos de letra no ficheiro".
- **Remover informações pessoais deste ficheiro ao guardar** retira o autor, a pessoa que guardou o ficheiro por último, a empresa, o gestor, bem como os nomes, as iniciais e as horas dos comentários, do ficheiro que é gravado (um ficheiro .pptx ou .odp, um modelo e uma apresentação enviada por correio eletrónico). O que o ficheiro diz mantém-se como está. As propriedades personalizadas do ficheiro são deixadas como estão, e o documento aberto não é alterado. É a mesma opção em todas as aplicações JeffOffice: se a alterar numa, altera-se em todas. A mesma caixa está na página **Informações**.

## Personalizar

O separador **Personalizar** tem três páginas: **Friso**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde escolhe as teclas de atalho dos comandos do JeffDeck. Veja "Escolher as suas próprias teclas de atalho" no tópico sobre o teclado.

### Personalizar o friso

A página **Friso** lista os separadores do friso, cada um com os seus grupos.

- **Ocultar ou mostrar um separador ou um grupo.** Desmarque a caixa ao lado para o ocultar, e marque-a para o voltar a mostrar. Um grupo oculto não ocupa espaço, e os seus comandos continuam a funcionar com as teclas de atalho. **Base** não tem caixa, porque está sempre visível, e o mesmo acontece com o botão **Ficheiro**. Um separador que só aparece quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar um separador ou grupo próprio.** Escreva um nome na caixa **Nome** e clique em **Novo Separador**. Para criar um grupo, escolha um separador e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer separador, incluindo um separador que só aparece quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (escreva na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Pode colocar qualquer comando que tenha nome. Um comando só pode estar num grupo uma vez.
- **Renomear.** Escolha um separador, grupo ou comando próprio, escreva o novo nome e clique em **Renomear**. Um comando que renomeia continua a fazer o mesmo, e o novo nome é apenas o texto do seu botão. Para renomear um comando com um nome vazio, e devolver-lhe o nome original.
- **Reordenar.** Escolha um separador e clique em **Mover para Cima** ou **Mover para Baixo**. Pode mover os separadores do próprio friso, bem como os seus, mas não os separadores que só aparecem quando algo está selecionado. Os seus grupos e comandos movem-se da mesma forma dentro do respetivo separador ou grupo. Os grupos do próprio friso mantêm a sua ordem.
- **Retirar.** Escolha um separador, grupo ou comando próprio e clique em **Remover**.

**Repor Um** repõe o separador escolhido: fica visível, os seus grupos ficam visíveis, e os grupos que lhe acrescentou desaparecem. Um separador próprio é removido. **Repor Tudo** repõe todo o friso, incluindo a ordem dos separadores.

Nada muda até clicar em **OK**. Depois, todas as janelas do JeffDeck abertas mostram o seu friso, e também todas as janelas que abrir mais tarde. O JeffDeck guarda apenas o que difere do friso habitual, por isso um separador ou grupo que uma versão posterior acrescente aparece por si. Os seus separadores e grupos recebem Sugestões de Teclas (as letras que aparecem ao premir **Alt**) que não entram em conflito com as outras. Pode ter 20 separadores próprios, 30 grupos próprios num separador e 60 comandos num grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** serve para a pequena barra de ferramentas no canto superior esquerdo da janela. A lista da esquerda mostra o que está atualmente na barra, e a lista **Comandos**, à direita, tem todos os comandos que têm nome (escreva na caixa **Pesquisar comandos** para a reduzir, do mesmo modo que na página **Friso**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Fica no fim da barra. Um comando que abre um menu mantém o seu menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Também pode clicar com o botão direito em qualquer comando do friso e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito num botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. No mesmo menu existem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar o Friso**, que abrem esta página.
- **Adicionar um separador.** Clique em **Adicionar Separador** para pôr uma linha fina no fim, e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Retirar.** Escolha-o e clique em **Remover**.

**Repor Um** repõe o comando escolhido: no sítio onde o JeffDeck normalmente o tem, ou fora da barra quando o JeffDeck não o costuma ter ali. **Repor Tudo** repõe a barra de ferramentas com que o JeffDeck vem. Repor Tudo nesta página não altera o seu friso, e Repor Tudo na página **Friso** não altera a sua barra de ferramentas.

Nada muda até clicar em **OK**. Depois, todas as janelas do JeffDeck abertas mostram a sua barra, e também todas as janelas que abrir mais tarde. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem num terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora do ecrã. O JeffDeck mantém na sua lista um comando que uma versão posterior já não tenha, e volta a mostrá-lo se o comando regressar. Os botões recebem Sugestões de Teclas, os números de 1 a 9 e depois duas letras, pela ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Ficheiro**.

## Relacionados

- [Separador Ver](tab-view)
- [Recuperar trabalho não guardado](recover)
- [O ecrã Ficheiro](file-screen)
