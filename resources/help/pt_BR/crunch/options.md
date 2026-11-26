# Alterar as configurações do JeffCrunch
<!-- keywords: opções de fórmulas, configurações de cálculo, alteração máxima, iteração, recuperação, recuperar trabalho não salvo, recuperação automática, revisão de texto, edição, preenchimento automático, alça de preenchimento, arrastar e soltar, casas decimais, opções, configurações, preferências, estatísticas de uso, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, excluir estatísticas, atualizações, sobre, editar na célula, cor das linhas de grade, zoom padrão, largura padrão, privacidade, informações pessoais, personalizar teclado, personalizar a faixa de opções, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar comando à barra de ferramentas rápida, ocultar guia, ocultar grupo, adicionar guia, adicionar grupo, renomear guia, redefinir a faixa de opções, alterar teclas de atalho, reatribuir teclas, redefinir teclas -->

Suas configurações ficam na janela **Opções do JeffCrunch**. Clique em **Arquivo > Opções** para abri-la. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Geral

**Mostrar Minibarra de Ferramentas na seleção** vem ativada por padrão. Ela define se a pequena barra de formatação aparece flutuando perto do texto selecionado e no topo do menu de atalho das células (veja [Digitar nas células](edit-cells)). A escolha é compartilhada pelos aplicativos do JeffOffice.

## Edição

A página **Edição** altera o modo como a digitação nas células funciona.

- **Após pressionar Enter, mover a seleção** define para onde a seleção vai quando você termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lugar nenhum.
- **Oferecer para concluir um texto a partir das células acima e abaixo** é chamado de Preenchimento Automático. Quando você digita as primeiras letras de um texto e apenas um texto daquela coluna começa com elas, o JeffCrunch mostra o restante, selecionado. Pressione **Enter** para mantê-lo. Continue digitando para ignorá-lo, ou pressione **Backspace** para removê-lo. Ele considera apenas o bloco de células ao lado daquela em que você está digitando, e apenas textos, não números.
- **Usar a alça de preenchimento e arrastar células para mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrasto pela borda da seleção. Desative-a se você move células sem querer. Copiar e Colar continuam funcionando.
- **Mostrar zero em células com valor zero (esta planilha)** liga ou desliga os zeros na planilha que você está vendo. Quando está desativada, uma célula com 0 parece vazia, mas continua contendo o 0 e é somada como 0. A escolha é salva com a planilha.
- **Inserir a vírgula decimal em números inteiros digitados** é chamado de casas decimais automáticas. Defina **Casas** como 2, e digitar 1234 insere 12,34. Defina como -2, e digitar 5 insere 500. Um número digitado com vírgula decimal é inserido como digitado.

- **Permitir edição diretamente nas células** vem ativada por padrão. Quando você a desativa, uma entrada digitada ou F2 vai para a barra de fórmulas, e a célula mostra o resultado somente quando você termina.

## Opções de fórmulas

A página **Fórmulas** define como a pasta de trabalho aberta calcula suas fórmulas: **Cálculo** (automático, automático exceto tabelas de dados, ou manual), se as fórmulas são calculadas antes de a pasta de trabalho ser salva quando o cálculo é manual, se uma fórmula que referencia a si mesma é calculada por iteração, o número máximo de iterações e a alteração que interrompe as iterações. Essas configurações pertencem à pasta de trabalho, não a você: são salvas no arquivo dela e não são mantidas para outras pastas de trabalho, e as opções ficam desativadas quando nenhuma pasta de trabalho está aberta. A alteração é mostrada com todos os seus dígitos, e você pode digitá-la como número, como 0,0000001 ou 1e-7. Somente uma configuração que você altera é gravada: **OK** sem nenhuma alteração não adiciona nenhum passo de Desfazer e deixa a pasta de trabalho como estava. Cada alteração é um passo de Desfazer. O arquivo de pasta de trabalho mais recente e o arquivo .xls mais antigo mantêm todas as configurações. Uma cópia em .ods mantém as configurações de iteração, mas não tem lugar para o cálculo manual; por isso, a cópia calcula as fórmulas à medida que mudam, e o relatório exibido antes de a cópia ser salva informa isso. As mesmas configurações estão na guia **Fórmulas**.

## Opções de salvamento

**Salvar informações de recuperação automática a cada** define com que frequência o JeffCrunch mantém uma cópia de uma pasta de trabalho com alterações que você não salvou, de 1 a 120 minutos. A cópia é um arquivo .xlsx em uma pasta que o JeffCrunch mantém para essa finalidade, feita em segundo plano para que você possa continuar digitando. Ela é removida quando você salva ou fecha a pasta de trabalho. Se o JeffCrunch fechar inesperadamente, ele oferece as cópias na próxima vez que for iniciado. Você também pode clicar em **Arquivo > Abrir > Recuperar Trabalho Não Salvo**. Uma pasta de trabalho protegida por senha não é copiada, pois a cópia não estaria protegida.

## Opções de revisão de texto

A página **Revisão de Texto** é compartilhada pelos aplicativos do JeffOffice. Ela define se a ortografia é verificada enquanto você digita em uma célula (desative **Verificar ortografia ao digitar** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nos outros aplicativos; **Ortografia** na guia Revisão continua funcionando), se palavras em letras maiúsculas e palavras com números são ignoradas na verificação ortográfica, se a Autocorreção corrige palavras enquanto você digita, se aspas retas viram aspas curvas, se travessões, frações e ordinais são alterados, e se um endereço da web vira um link. A guia **Símbolos Matemáticos** das Opções de Autocorreção é para o JeffWrite, o JeffDeck e o JeffPub: o JeffCrunch não transforma uma barra invertida e um nome em um símbolo em uma célula. Veja [Ortografia](notes-links) e [Digitar nas células](edit-cells).

## Opções de exibição

- **Cor das linhas de grade** é a cor das linhas entre as células na sua tela. É uma escolha apenas para a sua tela, não salva na pasta de trabalho.
- **Zoom de uma nova pasta de trabalho** define o tamanho com que as planilhas de uma nova pasta de trabalho aparecem, de 10% a 400%.
- **Largura padrão de coluna (esta planilha)** é a largura de todas as colunas desta planilha que não têm largura própria. É o mesmo que **Largura Padrão** na guia Página Inicial.
- **Mostrar as guias de planilha** e **Mostrar as barras de rolagem** ocultam ou exibem a linha de guias de planilha sob as células e as barras de rolagem à direita e embaixo. São escolhas apenas para a sua tela, mantidas para todas as pastas de trabalho que você abrir.
- **Mostrar a anotação de uma célula como** escolhe a aparência de uma célula com anotação: uma pequena marca na célula e a anotação quando o ponteiro para sobre ela, somente a marca, ou nada. Uma anotação que você escolheu mostrar o tempo todo (**Mostrar Anotação**) permanece na planilha em todos os casos.

## Personalizar

A guia **Personalizar** tem três páginas: **Faixa de Opções**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde você escolhe as teclas de atalho dos comandos do JeffCrunch. Veja "Escolher suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar a faixa de opções

A página **Faixa de Opções** lista as guias da faixa, cada uma com seus grupos.

- **Ocultar ou mostrar uma guia ou um grupo.** Desmarque a caixa ao lado para ocultá-lo, e marque a caixa para mostrá-lo novamente. Um grupo oculto não ocupa espaço, e seus comandos continuam funcionando pelas teclas de atalho. **Página Inicial** não tem caixa, porque sempre permanece, e o mesmo vale para o botão **Arquivo**. Uma guia que aparece somente quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar sua própria guia ou grupo.** Digite um nome na caixa **Nome** e clique em **Nova Guia**. Para criar um grupo, escolha uma guia e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer guia, inclusive em uma guia que aparece somente quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha o seu grupo, encontre um comando na lista **Comandos** (digite na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Qualquer comando que tenha nome pode entrar em um grupo. Um comando pode estar em um grupo apenas uma vez.
- **Renomear.** Escolha uma guia, um grupo ou um comando seu, digite o novo nome e clique em **Renomear**. Um comando renomeado faz a mesma coisa, e o novo nome é apenas o texto do botão. Para renomear um comando com nome vazio e devolver o nome original, faça o mesmo com o nome vazio.
- **Reordenar.** Escolha uma guia e clique em **Mover para Cima** ou **Mover para Baixo**. Você pode mover as guias da própria faixa e as suas, mas não as guias que aparecem somente quando algo está selecionado. Seus grupos e seus comandos se movem da mesma forma dentro da sua guia ou grupo. Os grupos da própria faixa mantêm a ordem.
- **Remover.** Escolha uma guia, um grupo ou um comando seu e clique em **Remover**.

**Redefinir Um** devolve a guia escolhida ao padrão: ela é exibida, seus grupos são exibidos, e os grupos que você acrescentou a ela são removidos. Uma guia sua é removida. **Redefinir Tudo** devolve a faixa inteira ao padrão, inclusive a ordem das guias.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffCrunch mostram a sua faixa, e também todas as janelas que você abrir depois. O JeffCrunch guarda apenas o que difere da faixa usual, então uma guia ou grupo que uma versão posterior acrescente aparece por conta própria. Suas guias e grupos recebem Dicas de Tecla (as letras exibidas quando você pressiona **Alt**) que não coincidem com as demais. Você pode ter 20 guias próprias, 30 grupos próprios em uma guia e 60 comandos em um grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** é para a pequena barra no canto superior esquerdo da janela. Sua lista à esquerda mostra o que está na barra agora, e a lista **Comandos** à direita tem todos os comandos que têm nome (digite na caixa **Pesquisar comandos** para reduzi-la, do mesmo modo que na página **Faixa de Opções**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Ele vai para o fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando pode estar na barra apenas uma vez.
- **Clique com o botão direito.** Você também pode clicar com o botão direito em qualquer comando da faixa e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito em um botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar a Faixa de Opções**, que abrem esta página.
- **Mover para abaixo da faixa.** Marque **Mostrar Barra de Ferramentas Rápida abaixo da faixa de opções** para colocar a barra em uma linha sob a faixa, em vez de no canto superior esquerdo. Os dois menus do clique com o botão direito também têm **Mostrar Barra de Ferramentas Rápida Abaixo da Faixa de Opções**. O JeffCrunch lembra a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim, e depois mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Redefinir Um** devolve o comando escolhido ao lugar em que o JeffCrunch normalmente o coloca, ou o tira da barra quando o JeffCrunch não costuma tê-lo ali. **Redefinir Tudo** devolve a barra que o JeffCrunch traz de fábrica. Redefinir Tudo nesta página não altera a sua faixa, e Redefinir Tudo na página **Faixa de Opções** não altera a sua barra.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffCrunch mostram a sua barra, e também todas as janelas que você abrir depois. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, assim o título da janela nunca sai da tela. O JeffCrunch mantém um comando na sua lista quando uma versão posterior não o tem mais, e o mostra novamente se o comando voltar. Os botões recebem Dicas de Tecla, os números de 1 a 9 e depois duas letras, na ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão Arquivo.

## Opções de privacidade

- **Remover informações pessoais deste arquivo ao salvar** retira o autor, a pessoa que salvou o arquivo por último, a empresa, o gerente, e os nomes e horários das anotações e comentários, do arquivo que é gravado (uma pasta de trabalho, uma cópia e uma pasta de trabalho enviada por email). O que o arquivo diz permanece como está. As propriedades personalizadas do arquivo são mantidas como estão, e o documento aberto não é alterado. É a mesma escolha em todos os aplicativos do JeffOffice: altere-a em um e ela muda em todos. Para fazer isso uma única vez, use **Inspecionar Pasta de Trabalho** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de uso

**Enviar estatísticas anônimas de uso** permite compartilhar dados sobre como o JeffCrunch é usado. Anônimo significa que os dados não dizem nada sobre quem você é. Quando esta opção está ativada, o JeffCrunch envia esses dados uma vez por dia, e no dia em que você atualiza: que é o JeffCrunch, a versão, o sistema operacional e o idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado, e que tipo de arquivo você abriu ou salvou. Se o JeffCrunch fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia seus arquivos, os nomes deles, nem nada que haja neles, nem o que você digitou. Se esta opção estiver desativada, nenhum relatório de falha é enviado.

A escolha vale para todos os aplicativos do JeffOffice: altere-a aqui e ela muda em cada um deles. O JeffCrunch pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele diz o que é enviado, para onde vai e por quanto tempo o guardamos. Guardamos as estatísticas por 24 meses.
- **Número de instalação** aparece abaixo da caixa de seleção. É um número aleatório gerado para a sua cópia do JeffOffice, o mesmo em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido gerado ainda, aparece **nenhum**.
- Para que as estatísticas que já temos sejam excluídas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador, e sem ele não conseguimos saber quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Segurança

A guia **Segurança** define o que o JeffCrunch faz com os itens para os quais um arquivo pode apontar: arquivos vinculados, filmes e sons vinculados, e arquivos da internet. As macros são mantidas no arquivo e nunca são executadas, e um link que iniciaria um programa é sempre recusado. Veja [Segurança](security).

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
