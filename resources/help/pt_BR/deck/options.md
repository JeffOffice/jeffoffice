# Alterar as configurações do JeffDeck
<!-- keywords: opções, configurações, preferências, tema, modo escuro, idioma, nome de usuário, iniciais, régua, ajustar, ortografia, autocorreção, aspas, atualizações, beta, estatísticas de uso, sobre, salvar, inserir fontes, fontes, personalizar teclado, personalizar a faixa de opções, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar comando à barra de ferramentas rápida, ocultar guia, ocultar grupo, adicionar guia, adicionar grupo, renomear guia, redefinir a faixa de opções, alterar teclas de atalho, reatribuir teclas, redefinir teclas -->

Suas configurações estão na janela **Opções do JeffDeck**. Clique em **Arquivo > Opções** para abri-la. Ela tem sete guias. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Geral

- **Nome de usuário** e **Iniciais** aparecem nos comentários que você escreve. Consulte [Comentários](comments).
- **Tema** é **Usar configuração do sistema**, **Claro** ou **Escuro**. A alteração aparece imediatamente.
- **Idioma de exibição** é o idioma das palavras do próprio JeffDeck. **Usar idioma do sistema** segue o idioma do seu computador. Outros idiomas aparecem nesta lista quando há traduções disponíveis. Um novo idioma começa a valer na próxima vez que você abrir o JeffDeck.
- **Verificar se há atualizações ao iniciar o JeffDeck** faz o JeffDeck procurar uma versão mais recente sempre que for iniciado. Se houver, a janela **Atualização disponível** mostra o que há de novo. Escolha **Atualizar agora**, **Depois** ou **Ignorar esta versão**. No Windows, **Atualizar agora** baixa o programa de instalação. Ele instala a nova versão de todos os aplicativos do JeffOffice e pede que você feche os que estiverem abertos. Em outros computadores, o botão é **Abrir página de download**. Ele abre a página onde você pode obter a nova versão. Se o JeffPub também estiver aberto, apenas um dos dois avisará sobre uma nova versão a cada dia.
- **Obter versões beta (compilações antecipadas para teste)** faz o JeffDeck oferecer também as versões beta. Elas são lançadas antes das versões regulares e podem ter imperfeições.
- **Enviar estatísticas anônimas de uso** permite compartilhar dados sobre como o JeffDeck é usado. Anônimo significa que os dados não revelam quem você é. Quando esta opção está ativada, o JeffDeck envia esses dados uma vez por dia e no dia em que você atualizar: que é o JeffDeck, sua versão, seu sistema operacional e idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou (por exemplo, .pptx). Se o JeffDeck for fechado inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O relatório indica em que ponto do programa ele parou, para que o problema possa ser encontrado e corrigido. O JeffDeck nunca envia seus arquivos, os nomes deles nem qualquer conteúdo, nem o que você digitou. Se esta opção estiver desativada, nenhum relatório de falha é enviado. O JeffDeck pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffDeck**. Ele não pergunta se você já fez essa escolha no JeffPub ou na instalação.
- **Aviso de privacidade**, ao lado dessa caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele explica o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** é mostrado abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, o mesmo em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número ainda tiver sido gerado, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffDeck, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Assim, nada é contabilizado nem enviado, e o JeffDeck não faz a pergunta da primeira execução.

**Nome de usuário**, **Iniciais**, **Tema** e **Idioma de exibição**, bem como as opções de atualização e estatísticas, valem para todos os aplicativos do JeffOffice: se você alterar uma delas aqui, ela muda em todos.

## Revisão de Texto

Estas caixas de seleção controlam a ortografia e a ajuda de digitação. Consulte [Digitar e formatar texto](text).

- **Verificar ortografia ao digitar**
- **Verificar Gramática Enquanto Você Digita** marca erros claros de gramática e de estilo em textos em inglês, francês, alemão e espanhol com uma linha pontilhada azul.
- **Configurações de Gramática** abre uma lista dos tipos de problemas a procurar: gramática, pontuação, uso de maiúsculas, palavras trocadas, repetição e estilo (que fica desativado até você ativá-lo).
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao digitar (AutoCorreção)** corrige erros comuns de digitação.
- **Substituir aspas retas por aspas inglesas** transforma " e ' em aspas curvas.
- **Formatar automaticamente ao digitar (travessões, frações, ordinais, listas automáticas)**
- **Transformar endereços da web em links conforme você digita** transforma um endereço da web ou de email em link quando você pressiona a barra de espaço depois dele. Desfazer remove apenas o link.

## Exibir

- **Mostrar régua** exibe as réguas nas bordas do slide. É o mesmo que **Exibir > Mostrar > Régua**, e vale para todas as janelas, as atuais e as que você abrir depois.
- **Ajustar objetos a outros objetos** faz com que um objeto arrastado se alinhe com os outros objetos do slide.

## Avançado

- **Usar a tecla Insert para ativar e desativar o modo de sobrescrita** vem desativada por padrão. Quando está ativada, a tecla **Insert** alterna entre dois modos de digitação em uma caixa de texto. Normalmente, cada letra digitada empurra as letras seguintes para a frente. No modo **sobrescrever**, cada letra digitada substitui a letra após o cursor, até o fim do parágrafo. A barra de status mostra **Inserir** ou **Sobrescrever** para indicar em qual modo você está. Pressione **Insert** novamente para voltar.

## Salvar

- **Inserir fontes no arquivo** coloca as fontes usadas nos slides dentro do arquivo ao salvar, para que os slides fiquem iguais em um computador que não tenha essas fontes. Essa opção fica ativada até você desmarcá-la. Ela inclui apenas as fontes que vêm com o JeffDeck e deixa o arquivo maior. O tópico Abrir e salvar apresentações traz os detalhes, em "Fontes no arquivo".
- **Remover informações pessoais deste arquivo ao salvar** retira o autor, quem salvou o arquivo por último, a empresa, o gerente e os nomes, as iniciais e os horários dos comentários do arquivo gravado (um arquivo .pptx ou .odp, um modelo e uma apresentação enviada por email). O conteúdo do arquivo permanece como está. As propriedades personalizadas do arquivo são mantidas como estão, e o documento aberto não é alterado. É a mesma escolha em todos os aplicativos do JeffOffice: alterá-la em um muda em todos. A mesma caixa está na página **Informações**.

## Personalizar

A guia **Personalizar** tem três páginas: **Faixa de Opções**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde você escolhe as teclas de atalho dos comandos do JeffDeck. Consulte "Escolher suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar a faixa de opções

A página **Faixa de Opções** lista as guias da faixa, cada uma com seus grupos.

- **Ocultar ou mostrar uma guia ou um grupo.** Desmarque a caixa ao lado para ocultá-lo e marque-a para mostrá-lo novamente. Um grupo oculto não ocupa espaço, e seus comandos continuam funcionando pelas teclas de atalho. **Página Inicial** não tem caixa, porque sempre permanece, e o botão **Arquivo** também. Uma guia que aparece somente quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar sua própria guia ou grupo.** Digite um nome na caixa **Nome** e clique em **Nova Guia**. Para criar um grupo, escolha uma guia e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer guia, inclusive em uma guia que aparece somente quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha seu grupo, encontre um comando na lista **Comandos** (digite na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Qualquer comando que tenha nome pode entrar em um grupo. Um comando pode estar em um grupo apenas uma vez.
- **Renomear.** Escolha uma guia, um grupo ou um comando próprio, digite o novo nome e clique em **Renomear**. Um comando renomeado faz a mesma coisa, e o novo nome aparece apenas no texto do botão. Para renomear um comando com nome vazio e devolver o nome original dele, use a mesma opção.
- **Reordenar.** Escolha uma guia e clique em **Mover para Cima** ou **Mover para Baixo**. Você pode mover as guias da própria faixa, assim como as suas, mas não as guias que aparecem somente quando algo está selecionado. Seus grupos e seus comandos se movem da mesma forma dentro da guia ou do grupo. Os grupos da própria faixa mantêm a ordem.
- **Remover.** Escolha uma guia, um grupo ou um comando próprio e clique em **Remover**.

**Redefinir Um** restaura a guia escolhida: ela volta a aparecer, seus grupos voltam a aparecer, e os grupos que você acrescentou a ela são removidos. Uma guia própria é excluída. **Redefinir Tudo** restaura a faixa inteira, inclusive a ordem das guias.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffDeck mostram sua faixa, assim como todas as janelas que você abrir depois. O JeffDeck guarda apenas o que difere da faixa usual, de modo que uma guia ou grupo que uma versão posterior acrescentar aparecerá por conta própria. Suas guias e grupos recebem Dicas de Tecla (as letras mostradas ao pressionar **Alt**) que não se confundem com as demais. Você pode ter 20 guias próprias, 30 grupos próprios em uma guia e 60 comandos em um grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** é para a pequena barra no canto superior esquerdo da janela. Sua lista à esquerda mostra o que está na barra agora, e a lista **Comandos**, à direita, tem todos os comandos que têm nome (digite na caixa **Pesquisar comandos** para reduzi-la, do mesmo modo que na página **Faixa de Opções**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Ele vai para o fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando pode estar na barra apenas uma vez.
- **Clique com o botão direito.** Você também pode clicar com o botão direito em qualquer comando da faixa e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito em um botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar a Faixa de Opções**, que abrem esta página.
- **Mover para baixo da faixa.** Marque **Mostrar Barra de Ferramentas Rápida abaixo da faixa de opções** para colocar a barra em uma linha sob a faixa, em vez de no canto superior esquerdo. Os dois menus do clique com o botão direito também têm **Mostrar Barra de Ferramentas Rápida Abaixo da Faixa de Opções**. O JeffDeck lembra essa escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e, depois, mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista à esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Redefinir Um** restaura o comando escolhido: no lugar em que o JeffDeck normalmente o coloca ou fora da barra, quando o JeffDeck não o costuma ter ali. **Redefinir Tudo** restaura a barra que vem com o JeffDeck. Redefinir Tudo nesta página não altera sua faixa, e Redefinir Tudo na página **Faixa de Opções** não altera sua barra.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffDeck mostram sua barra, assim como todas as janelas que você abrir depois. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora da tela. O JeffDeck mantém um comando na sua lista quando uma versão posterior não o tem mais e o mostra novamente se ele voltar. Os botões recebem Dicas de Tecla, os números de 1 a 9 e depois duas letras, na ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Arquivo**.

## Segurança

A guia **Segurança** decide o que o JeffDeck faz com os itens para os quais um arquivo pode apontar: arquivos vinculados, filmes e sons vinculados e arquivos da internet. As macros são mantidas no arquivo e nunca são executadas, e um link que iniciaria um programa é sempre recusado. Consulte [Segurança](security).

## Relacionados

- [Guia Exibir](tab-view)
- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
