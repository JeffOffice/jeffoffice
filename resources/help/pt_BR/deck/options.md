# Alterar as configurações do JeffDeck
<!-- keywords: opções, configurações, preferências, tema, modo escuro, idioma, nome de usuário, iniciais, régua, ajustar, ortografia, autocorreção, aspas, atualizações, beta, estatísticas de uso, sobre, salvar, inserir fontes, fontes, personalizar teclado, personalizar a faixa de opções, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar comando à barra de ferramentas rápida, ocultar guia, ocultar grupo, adicionar guia, adicionar grupo, renomear guia, redefinir a faixa de opções, alterar teclas de atalho, reatribuir teclas, redefinir teclas -->

Suas configurações estão na janela **Opções do JeffDeck**. Clique em **Arquivo > Opções** para abri-la. Ela tem sete guias. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Geral

- **Nome de usuário** e **Iniciais** aparecem nos comentários que você escreve. Veja [Comentários](comments).
- **Tema** é **Usar configuração do sistema**, **Claro** ou **Escuro**. A alteração aparece imediatamente.
- **Idioma de exibição** é o idioma das palavras do próprio JeffDeck. **Usar idioma do sistema** segue o seu computador. Outros idiomas aparecem nesta lista quando há traduções disponíveis. Um novo idioma começa a valer na próxima vez que você abrir o JeffDeck.
- **Verificar se há atualizações ao iniciar o JeffDeck** faz o JeffDeck procurar uma versão mais nova sempre que ele é iniciado. Se houver, uma janela **Atualização disponível** mostra o que há de novo. Escolha **Atualizar agora**, **Depois** ou **Ignorar esta versão**. No Windows, **Atualizar agora** baixa o programa de instalação. Ele instala a nova versão de todos os aplicativos do JeffOffice e pede que você feche os que estiverem abertos. Em outros computadores, o botão é **Abrir página de download**. Ele abre a página onde você pode obter a nova versão. Se o JeffPub também estiver aberto, apenas um dos dois avisa sobre uma nova versão por dia.
- **Obter versões beta (compilações antecipadas para teste)** faz o JeffDeck oferecer também as versões beta. Elas são lançadas antes das versões regulares e podem ter imperfeições.
- **Enviar estatísticas anônimas de uso** permite compartilhar dados sobre como o JeffDeck é usado. Anônimo significa que os dados não revelam quem você é. Quando isto está ativado, o JeffDeck envia esses dados uma vez por dia e no dia em que você atualizar: que é o JeffDeck, a versão, o sistema operacional e o idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou (por exemplo, .pptx). Se o JeffDeck fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O relatório indica em que ponto do programa ele parou, para que o problema possa ser encontrado e corrigido. O JeffDeck nunca envia seus arquivos, os nomes deles nem nada do que há neles, nem o que você digitou. Se esta opção estiver desativada, nenhum relatório de falha é enviado. O JeffDeck pergunta sobre isto na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffDeck**. Ele não pergunta se você já escolheu no JeffPub ou na instalação.
- **Aviso de privacidade**, ao lado dessa caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele informa o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** é exibido abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, o mesmo em todos os aplicativos do JeffOffice. Ele não revela nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número ainda tiver sido gerado, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffDeck, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Nada é então contabilizado nem enviado, e o JeffDeck não faz a pergunta da primeira execução.

**Nome de usuário**, **Iniciais**, **Tema** e **Idioma de exibição**, assim como as opções de atualização e de estatísticas, valem para todos os aplicativos do JeffOffice: altere uma delas aqui e ela muda em todos.

## Revisão de texto

Estas caixas de seleção controlam a verificação ortográfica e a ajuda para digitação. Veja [Digitar e formatar texto](text).

- **Verificar ortografia ao digitar**
- **Verificar gramática enquanto você digita** marca erros claros de gramática e de estilo em texto em inglês, em todas as variantes regionais, com uma linha pontilhada azul. Textos em outros idiomas são verificados apenas quanto à ortografia.
- **Configurações de gramática** abre uma lista dos tipos de problemas a procurar: gramática, pontuação, uso de maiúsculas, palavras trocadas, repetição e estilo (que fica desativado até você ativá-lo).
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao digitar (AutoCorreção)** corrige erros comuns de digitação. Em **Opções de Autocorreção**, a guia **Símbolos Matemáticos** tem uma chave, **Substituir os nomes de símbolos matemáticos ao digitar**, que fica desativada até você ativá-la. Com ela ativada, uma barra invertida seguida de um nome, como \alpha, vira o símbolo (α) quando você digita um espaço ou um sinal depois dele. O nome precisa começar uma palavra, de modo que um caminho de pasta como C:\temp\pi permanece como você digitou. A guia lista todos os nomes. O JeffDeck, o JeffWrite e o JeffPub fazem essas substituições; o JeffCrunch não altera nada em uma célula.
- **Substituir aspas retas por aspas inglesas** troca " e ' por aspas curvas.
- **Formatar automaticamente ao digitar (travessões, frações, ordinais, listas automáticas)**
- **Transformar endereços da web em links conforme você digita** transforma um endereço da web ou de e-mail em link quando você pressiona a barra de espaço depois dele. Desfazer remove apenas o link.

## Exibir

- **Mostrar a régua** exibe as réguas nas bordas do slide. É o mesmo que **Exibir > Mostrar > Régua**, e vale para todas as janelas, as já abertas e as que você abrir depois.
- **Ajustar objetos a outros objetos** faz com que um objeto que você arrasta se alinhe com os outros objetos do slide.

## Avançado

- **Usar a tecla Insert para ativar e desativar o modo de sobrescrita** vem desativada por padrão. Quando ativada, a tecla **Insert** alterna entre duas formas de digitar em uma caixa de texto. Normalmente, cada letra que você digita empurra as letras seguintes. No modo **sobrescrita**, cada letra digitada substitui a letra à direita do cursor, até o fim do parágrafo. A barra de status mostra **Inserir** ou **Sobrescrever**, para você saber em qual modo está. Pressione **Insert** de novo para voltar.

## Salvar

- **Inserir fontes no arquivo** coloca dentro do arquivo, ao salvar, as fontes usadas nos seus slides, para que eles tenham a mesma aparência em um computador que não tenha essas fontes. Esta opção fica ativada até você desmarcá-la. Ela inclui apenas as fontes que acompanham o JeffDeck e aumenta o tamanho do arquivo. O tópico Abrir e salvar apresentações traz os detalhes, na seção "Fontes no arquivo".
- **Remover informações pessoais deste arquivo ao salvar** retira do arquivo gravado (um arquivo .pptx ou .odp, um modelo e uma apresentação enviada por e-mail) o autor, a pessoa que salvou o arquivo por último, a empresa, o gerente e os nomes, as iniciais e os horários dos comentários. O que o arquivo diz permanece como está. As propriedades personalizadas do arquivo são mantidas e o documento aberto não é alterado. É a mesma escolha em todos os aplicativos do JeffOffice: altere em um e ela muda em todos. A mesma caixa está na página **Informações**.

## Personalizar

A guia **Personalizar** tem três páginas: **Faixa de Opções**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde você escolhe as teclas de atalho dos comandos do JeffDeck. Veja "Escolher suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar a faixa de opções

A página **Faixa de Opções** lista as guias da faixa, cada uma com seus grupos.

- **Ocultar ou mostrar uma guia ou um grupo.** Desmarque a caixa ao lado para ocultá-lo e marque-a para mostrá-lo de novo. Um grupo oculto não ocupa espaço, e seus comandos continuam funcionando pelas teclas de atalho. **Página Inicial** não tem caixa, porque sempre permanece, assim como o botão **Arquivo**. Uma guia que aparece somente quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar sua própria guia ou grupo.** Digite um nome na caixa **Nome** e clique em **Nova Guia**. Para criar um grupo, escolha uma guia e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer guia, inclusive em uma guia que aparece somente quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (digite na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Qualquer comando que tenha um nome pode entrar em um grupo. Um comando só pode estar em um grupo uma vez.
- **Renomear.** Escolha uma guia, grupo ou comando seu, digite o novo nome e clique em **Renomear**. Um comando renomeado faz a mesma coisa, e o novo nome aparece apenas no texto do seu botão. Para renomear um comando com um nome vazio, e devolver-lhe o próprio nome, faça o mesmo com esse nome vazio.
- **Reordenar.** Escolha uma guia e clique em **Mover para Cima** ou **Mover para Baixo**. Você pode mover as guias da própria faixa, assim como as suas, mas não as guias que aparecem somente quando algo está selecionado. Seus grupos e seus comandos se movem da mesma forma dentro da sua guia ou grupo. Os grupos da própria faixa mantêm a ordem.
- **Remover.** Escolha uma guia, grupo ou comando seu e clique em **Remover**.

**Redefinir Um** restaura a guia escolhida: ela volta a ser mostrada, seus grupos voltam a ser mostrados e os grupos que você acrescentou a ela são removidos. Uma guia sua é excluída. **Redefinir Tudo** restaura a faixa inteira, inclusive a ordem das guias.

Nada muda até que você clique em **OK**. Depois disso, todas as janelas abertas do JeffDeck mostram a sua faixa, assim como qualquer janela que você abrir depois. O JeffDeck guarda apenas o que difere da faixa usual, de modo que uma guia ou grupo que uma versão posterior acrescentar aparece automaticamente. Suas guias e grupos recebem Dicas de Tecla (as letras mostradas quando você pressiona **Alt**) que não se repetem com as outras. Você pode ter 20 guias próprias, 30 grupos próprios em uma guia e 60 comandos em um grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** trata da pequena barra no canto superior esquerdo da janela. A lista da esquerda mostra o que está na barra agora, e a lista **Comandos** à direita tem todos os comandos que têm nome (digite na caixa **Pesquisar comandos** para reduzi-la, do mesmo modo que na página **Faixa de Opções**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Ele vai para o fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Você também pode clicar com o botão direito em qualquer comando da faixa e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito em um botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar a Faixa de Opções**, que abrem esta página.
- **Mover para baixo da faixa.** Marque **Mostrar Barra de Ferramentas Rápida abaixo da faixa de opções** para colocar a barra em uma linha sob a faixa, em vez de no canto superior esquerdo. Os dois menus de clique com o botão direito também têm **Mostrar Barra de Ferramentas Rápida Abaixo da Faixa de Opções**. O JeffDeck lembra a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Redefinir Um** restaura o comando escolhido: no lugar em que o JeffDeck normalmente o coloca ou, se ele normalmente não fica na barra, fora dela. **Redefinir Tudo** restaura a barra com que o JeffDeck vem. Redefinir Tudo nesta página não altera a sua faixa, e Redefinir Tudo na página **Faixa de Opções** não altera a sua barra.

Nada muda até que você clique em **OK**. Depois disso, todas as janelas abertas do JeffDeck mostram a sua barra, assim como qualquer janela que você abrir depois. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, de modo que o título da janela nunca é empurrado para fora da tela. O JeffDeck mantém um comando na sua lista quando uma versão posterior não o tem mais, e volta a mostrá-lo se ele reaparecer. Os botões recebem Dicas de Tecla, os números de 1 a 9 e depois duas letras, na ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Arquivo**.

## Segurança

A guia **Segurança** decide o que o JeffDeck faz com as coisas para as quais um arquivo pode apontar: arquivos vinculados, filmes e sons vinculados e arquivos da internet. As macros são guardadas no arquivo e nunca são executadas, e um link que iniciaria um programa é sempre recusado. Veja [Segurança](security).

## Relacionados

- [Guia Exibir](tab-view)
- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
