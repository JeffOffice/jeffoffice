# Alterar as configurações do JeffDeck

<!-- keywords: opções, configurações, preferências, tema, modo escuro, idioma, nome de usuário, iniciais, régua, ajustar, ortografia, autocorreção, aspas, atualizações, beta, estatísticas de uso, sobre, salvar, inserir fontes, fontes, personalizar teclado, personalizar a faixa de opções, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar comando à barra de ferramentas rápida, ocultar guia, ocultar grupo, adicionar guia, adicionar grupo, renomear guia, redefinir a faixa de opções, alterar teclas de atalho, reatribuir teclas, redefinir teclas -->

Suas configurações ficam na janela **Opções do JeffDeck**. Clique em **Arquivo > Opções** para abri-la. Ela tem cinco guias. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Geral

- **Nome de usuário** e **Iniciais** aparecem nos comentários que você escreve. Veja [Comentários](comments).
- **Tema** é **Usar configuração do sistema**, **Claro** ou **Escuro**. A alteração aparece imediatamente.
- **Idioma de exibição** é o idioma das palavras do próprio JeffDeck. **Usar idioma do sistema** segue o seu computador. Outros idiomas aparecem nesta lista quando as traduções estão disponíveis. Um novo idioma começa a valer na próxima vez que você abrir o JeffDeck.
- **Verificar se há atualizações ao iniciar o JeffDeck** faz o JeffDeck procurar uma versão mais nova toda vez que é iniciado. Se houver uma, a janela **Atualização disponível** mostra o que há de novo. Escolha **Atualizar agora**, **Mais tarde** ou **Ignorar esta versão**. No Windows, **Atualizar agora** baixa o programa de instalação. Ele instala a nova versão de todos os aplicativos do JeffOffice e pede que você feche os que estiverem abertos. Em outros computadores, o botão é **Abrir página de download**. Ele abre a página em que você pode obter a nova versão. Se o JeffPub também estiver aberto, apenas um dos dois avisará sobre uma nova versão por dia.
- **Obter versões beta (compilações antecipadas para teste)** faz o JeffDeck oferecer também as versões beta. Elas são lançadas antes das versões regulares e podem ter problemas.
- **Enviar estatísticas anônimas de uso** permite compartilhar dados sobre como o JeffDeck é usado. Anônimo significa que os dados não dizem nada sobre você. Quando essa opção está ativada, o JeffDeck envia esses dados uma vez por dia e no dia em que você atualizar: que é o JeffDeck, sua versão, o sistema operacional e o idioma, quantas vezes foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou (por exemplo, .pptx). Se o JeffDeck fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O relatório indica em que ponto do programa ele parou, para que o problema possa ser encontrado e corrigido. O JeffDeck nunca envia seus arquivos, os nomes deles, nem qualquer conteúdo, nem o que você digitou. Se essa opção estiver desativada, nenhum relatório de falha é enviado. O JeffDeck pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffDeck**. Ele não pergunta se você já escolheu no JeffPub ou na instalação.
- **Aviso de privacidade**, ao lado dessa caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele informa o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** aparece abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, igual em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido gerado ainda, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffDeck, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Assim, nada é contado nem enviado, e o JeffDeck não faz a pergunta da primeira execução.

**Nome de usuário**, **Iniciais**, **Tema** e **Idioma de exibição**, além das opções de atualização e de estatísticas, valem para todos os aplicativos do JeffOffice: alterar uma aqui muda em cada um deles.

## Revisão de texto

Estas caixas de seleção controlam a ortografia e a ajuda na digitação. Veja [Digitar e formatar texto](text).

- **Verificar ortografia ao digitar**
- **Ignorar palavras em MAIÚSCULAS**
- **Ignorar palavras que contêm números**
- **Substituir texto ao digitar (AutoCorreção)** corrige erros comuns de digitação.
- **Substituir aspas retas por aspas inglesas** transforma " e ' em aspas curvas.
- **Formatar automaticamente ao digitar (travessões, frações, ordinais, listas automáticas)**
- **Transformar endereços da web em links conforme você digita** transforma um endereço da web ou de email em link quando você pressiona a barra de espaço depois dele. Desfazer remove apenas o link.

## Exibir

- **Mostrar réguas** exibe as réguas nas bordas do slide. É o mesmo que **Exibir > Mostrar > Régua**, e vale para todas as janelas, as abertas agora e as que você abrir depois.
- **Ajustar objetos a outros objetos** faz um objeto que você arrasta se alinhar aos outros objetos do slide.

## Salvar

- **Inserir fontes no arquivo** coloca no arquivo, ao salvar, as fontes usadas nos slides, para que eles tenham a mesma aparência em um computador que não tenha essas fontes. Essa opção fica ativada até você desmarcá-la. Ela inclui apenas as fontes que acompanham o JeffDeck e aumenta o tamanho do arquivo. O tópico Abrir e salvar apresentações traz os detalhes, na seção "Fontes no arquivo".
- **Remover informações pessoais deste arquivo ao salvar** retira do arquivo gravado (um arquivo .pptx ou .odp, um modelo e uma apresentação enviada por email) o autor, a pessoa que salvou o arquivo por último, a empresa, o gerente, além dos nomes, iniciais e horários dos comentários. O conteúdo do arquivo permanece como está. As propriedades personalizadas do arquivo são mantidas, e o documento aberto não é alterado. É a mesma opção em todos os aplicativos do JeffOffice: alterá-la em um muda em todos eles. A mesma caixa está na página **Informações**.

## Personalizar

A guia **Personalizar** tem três páginas: **Faixa de Opções**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde você escolhe as teclas de atalho dos comandos do JeffDeck. Veja "Escolher suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar a faixa de opções

A página **Faixa de Opções** lista as guias da faixa, cada uma com seus grupos.

- **Ocultar ou mostrar uma guia ou um grupo.** Desmarque a caixa ao lado para ocultá-lo e marque-a para mostrá-lo novamente. Um grupo oculto não ocupa espaço, e seus comandos continuam funcionando pelas teclas de atalho. **Página Inicial** não tem caixa, porque sempre permanece, e o botão **Arquivo** também. Uma guia que aparece apenas quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar sua própria guia ou grupo.** Digite um nome na caixa **Nome** e clique em **Nova Guia**. Para criar um grupo, escolha uma guia e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer guia, inclusive em uma guia que aparece apenas quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha seu grupo, encontre um comando na lista **Comandos** (digite na caixa **Pesquisar comandos** para restringir a lista) e clique em **Adicionar Comando**. Qualquer comando com nome pode entrar no grupo. Um comando só pode estar uma vez em cada grupo.
- **Renomear.** Escolha uma guia, grupo ou comando próprio, digite o novo nome e clique em **Renomear**. Um comando renomeado faz a mesma coisa, e o novo nome é apenas o que o botão mostra. Para dar a um comando o nome original, renomeie-o com um nome vazio.
- **Reordenar.** Escolha uma guia e clique em **Mover para Cima** ou **Mover para Baixo**. Você pode mover as guias da própria faixa, assim como as suas, mas não as guias que aparecem apenas quando algo está selecionado. Seus grupos e comandos se movem da mesma forma dentro da guia ou do grupo. Os grupos da própria faixa mantêm a ordem.
- **Remover.** Escolha uma guia, grupo ou comando próprio e clique em **Remover**.

**Redefinir Um** restaura a guia escolhida: ela passa a ser exibida, seus grupos também, e os grupos que você acrescentou a ela são removidos. Uma guia criada por você é excluída. **Redefinir Tudo** restaura a faixa inteira, inclusive a ordem das guias.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffDeck mostram sua faixa, assim como qualquer janela aberta depois. O JeffDeck guarda apenas o que difere da faixa padrão, então uma guia ou grupo que uma versão futura acrescentar aparece automaticamente. Suas guias e grupos recebem Dicas de Tecla (as letras exibidas ao pressionar **Alt**) que não se repetem com as demais. Você pode ter 20 guias próprias, 30 grupos próprios em uma guia e 60 comandos em um grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** cuida da pequena barra no canto superior esquerdo da janela. Sua lista à esquerda mostra o que está na barra agora, e a lista **Comandos** à direita tem todos os comandos com nome (digite na caixa **Pesquisar comandos** para restringi-la, do mesmo modo que na página **Faixa de Opções**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Ele vai para o fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar uma vez na barra.
- **Clique com o botão direito.** Você também pode clicar com o botão direito em qualquer comando da faixa e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito em um botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar a Faixa de Opções**, que abrem esta página.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Redefinir Um** restaura o comando escolhido: no lugar onde o JeffDeck normalmente o coloca, ou fora da barra, quando o JeffDeck normalmente não o tem ali. **Redefinir Tudo** restaura a barra com que o JeffDeck vem. Redefinir Tudo nesta página não altera sua faixa, e Redefinir Tudo na página **Faixa de Opções** não altera sua barra.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffDeck mostram sua barra, assim como qualquer janela aberta depois. A barra pode ter, no total, 100 comandos e separadores. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca saia da tela. O JeffDeck mantém na sua lista um comando que uma versão posterior não tenha mais, e o exibe novamente se ele voltar. Os botões recebem Dicas de Tecla, os números de 1 a 9 e depois duas letras, na ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Arquivo**.

## Relacionados

- [Guia Exibir](tab-view)
- [Recuperar trabalho não salvo](recover)
- [A tela Arquivo](file-screen)
