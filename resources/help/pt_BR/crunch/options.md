# Alterar as configurações do JeffCrunch
<!-- keywords: recuperação, recuperar trabalho não salvo, recuperação automática, revisão de texto, edição, preenchimento automático, alça de preenchimento, arrastar e soltar, casas decimais, opções, configurações, preferências, estatísticas de uso, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, excluir estatísticas, atualizações, sobre, editar na célula, cor das linhas de grade, zoom padrão, largura padrão, informações pessoais, personalizar teclado, personalizar a faixa de opções, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar comando à barra de ferramentas rápida, ocultar guia, ocultar grupo, adicionar guia, adicionar grupo, renomear guia, redefinir a faixa de opções, alterar teclas de atalho, reatribuir teclas, redefinir teclas -->

Suas configurações ficam na janela **Opções do JeffCrunch**. Clique em **Arquivo > Opções** para abri-la. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Geral

**Mostrar Minibarra de Ferramentas na seleção** vem ativada por padrão. Ela define se a pequena barra de formatação aparece flutuando perto do texto selecionado e no topo do menu de atalho das células (veja [Digitar nas células](edit-cells)). A opção é compartilhada pelos aplicativos do JeffOffice.

## Edição

A página **Edição** altera como a digitação nas células funciona.

- **Após pressionar Enter, mover a seleção** define para onde a seleção vai quando você conclui uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lugar nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** é chamada de AutoCompletar. Quando você digita as primeiras letras de um texto e apenas um texto daquela coluna começa com elas, o JeffCrunch mostra o restante, selecionado. Pressione **Enter** para mantê-lo. Continue digitando para ignorá-lo ou pressione **Backspace** para removê-lo. Ele considera apenas o intervalo de células ao lado daquela em que você está digitando, e apenas textos, não números.
- **Usar a alça de preenchimento e arrastar células para mover ou copiar** ativa o pequeno quadrado no canto da seleção e a possibilidade de arrastar a borda da seleção. Desative-o se você move células por engano. Copiar e Colar continuam funcionando.
- **Mostrar zero em células que têm valor zero (esta planilha)** ativa ou desativa os zeros na planilha que você está vendo. Quando está desativado, uma célula que contém 0 parece vazia, mas continua contendo o 0 e é somada como 0. A escolha é salva com a planilha.
- **Colocar um ponto decimal em números inteiros digitados** é chamado de casas decimais automáticas. Defina **Casas** como 2, e digitar 1234 insere 12,34. Defina como -2, e digitar 5 insere 500. Um número digitado com vírgula decimal é inserido como foi digitado.

- **Permitir edição diretamente nas células** vem ativada por padrão. Quando você a desativa, uma entrada digitada ou F2 vai para a barra de fórmulas, e a célula mostra o resultado somente quando você conclui a entrada.

## Opções de salvamento

**Salvar informações de recuperação automática a cada** define de quanto em quanto tempo o JeffCrunch guarda uma cópia de uma pasta de trabalho com alterações que você ainda não salvou, de 1 a 120 minutos. A cópia é um arquivo .xlsx em uma pasta que o JeffCrunch mantém para esse fim, feita em segundo plano para que você possa continuar digitando. Ela é removida quando você salva ou fecha a pasta de trabalho. Se o JeffCrunch fechar inesperadamente, ele oferece as cópias na próxima vez que for iniciado. Você também pode clicar em **Arquivo > Abrir > Recuperar Trabalho Não Salvo**. Uma pasta de trabalho protegida com senha não é copiada, porque a cópia não ficaria protegida.

## Opções de revisão de texto

A página **Revisão de Texto** é compartilhada pelos aplicativos do JeffOffice. Ela define se a ortografia é verificada enquanto você digita em uma célula (desative **Verificar ortografia ao digitar** para eliminar os sublinhados vermelhos ondulados em todas as janelas abertas e nos outros aplicativos; **Ortografia** na guia Revisão continua funcionando), se palavras em letras maiúsculas e palavras com números são ignoradas na verificação ortográfica, se a Autocorreção corrige palavras enquanto você digita, se aspas retas viram aspas curvas, se travessões, frações e ordinais são alterados e se um endereço da web vira um link. Veja [Ortografia](notes-links) e [Digitar nas células](edit-cells).

## Opções de exibição

- **Cor das linhas de grade** é a cor das linhas entre as células na sua tela. É uma escolha apenas para a sua tela, e não é salva na pasta de trabalho.
- **Zoom de uma nova pasta de trabalho** define o tamanho com que as planilhas de uma nova pasta de trabalho aparecem, de 10% a 400%.
- **Largura padrão da coluna (esta planilha)** é a largura de todas as colunas desta planilha que não têm largura própria. É o mesmo que **Largura Padrão** na guia Página Inicial.

## Personalizar

A guia **Personalizar** tem três páginas: **Faixa de Opções**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde você escolhe as teclas de atalho dos comandos do JeffCrunch. Veja "Escolher suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar a faixa de opções

A página **Faixa de Opções** lista as guias da faixa, cada uma com seus grupos.

- **Ocultar ou mostrar uma guia ou um grupo.** Desmarque a caixa ao lado para ocultá-lo e marque-a para mostrá-lo de novo. Um grupo oculto não ocupa espaço, e seus comandos continuam funcionando pelas teclas de atalho. **Página Inicial** não tem caixa, porque sempre permanece, e o botão **Arquivo** também. Uma guia que aparece somente quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar sua própria guia ou grupo.** Digite um nome na caixa **Nome** e clique em **Nova Guia**. Para criar um grupo, escolha uma guia e clique em **Novo Grupo**. Um grupo próprio pode ficar em qualquer guia, inclusive em uma guia que aparece somente quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha seu grupo, encontre um comando na lista **Comandos** (digite na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Qualquer comando que tenha nome pode entrar em um grupo. Um comando só pode aparecer uma vez em um grupo.
- **Renomear.** Escolha uma guia, grupo ou comando próprio, digite o novo nome e clique em **Renomear**. Um comando renomeado faz a mesma coisa, e o novo nome é apenas o que o botão mostra. Para dar a um comando o seu nome original de volta, renomeie-o com um nome vazio.
- **Reordenar.** Escolha uma guia e clique em **Mover para Cima** ou **Mover para Baixo**. Você pode mover as guias da própria faixa, assim como as suas, mas não as guias que aparecem somente quando algo está selecionado. Seus grupos e seus comandos se movem da mesma forma dentro da sua guia ou grupo. Os grupos da própria faixa mantêm a ordem.
- **Remover.** Escolha uma guia, grupo ou comando próprio e clique em **Remover**.

**Redefinir Um** devolve a guia escolhida ao normal: ela volta a aparecer, seus grupos voltam a aparecer e os grupos que você acrescentou a ela são removidos. Uma guia própria é removida. **Redefinir Tudo** devolve a faixa inteira ao normal, inclusive a ordem das guias.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffCrunch mostram a sua faixa, assim como qualquer janela que você abrir depois. O JeffCrunch guarda apenas o que difere da faixa usual, então uma guia ou um grupo que uma versão posterior acrescente aparece por conta própria. Suas guias e grupos recebem Dicas de Tecla (as letras mostradas quando você pressiona **Alt**) que não se repetem com as demais. Você pode ter 20 guias próprias, 30 grupos próprios em uma guia e 60 comandos em um grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** é para a pequena barra no canto superior esquerdo da janela. Sua lista à esquerda mostra o que está na barra agora, e a lista **Comandos** à direita tem todos os comandos que têm nome (digite na caixa **Pesquisar comandos** para reduzi-la, da mesma forma que na página **Faixa de Opções**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Ele vai para o fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando só pode estar na barra uma vez.
- **Clique com o botão direito.** Você também pode clicar com o botão direito em qualquer comando da faixa e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito em um botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar a Faixa de Opções**, que abrem esta página.
- **Mover a barra para abaixo da faixa.** Marque **Mostrar Barra de Ferramentas Rápida abaixo da faixa de opções** para colocar a barra em uma linha sob a faixa, em vez de no canto superior esquerdo. Os dois menus de clique com o botão direito também têm **Mostrar Barra de Ferramentas Rápida Abaixo da Faixa de Opções**. O JeffCrunch lembra a escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e depois a mova para onde quiser.
- **Reordenar.** Escolha um comando ou um separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Redefinir Um** devolve o comando escolhido ao normal: para onde o JeffCrunch costuma colocá-lo, ou para fora da barra quando o JeffCrunch não costuma tê-lo ali. **Redefinir Tudo** devolve a barra com que o JeffCrunch é fornecido. Redefinir Tudo nesta página não altera sua faixa de opções, e Redefinir Tudo na página **Faixa de Opções** não altera sua barra.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffCrunch mostram a sua barra, assim como qualquer janela que você abrir depois. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca seja empurrado para fora da tela. O JeffCrunch mantém na sua lista um comando que uma versão posterior não tem mais, e o mostra novamente se ele voltar. Os botões recebem Dicas de Tecla: os números de 1 a 9 e depois duas letras, na ordem em que aparecem na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Arquivo**.

## Opções de privacidade

- **Remover informações pessoais deste arquivo ao salvar** retira o autor, a pessoa que salvou o arquivo por último, a empresa, o gerente e os nomes e horários de anotações e comentários do arquivo que é gravado (uma pasta de trabalho, uma cópia e uma pasta de trabalho enviada por email). O que o arquivo diz permanece como está. As propriedades personalizadas do arquivo são mantidas, e o documento aberto não é alterado. É a mesma opção em todos os aplicativos do JeffOffice: altere em um e ela muda em todos. Para fazer isso apenas uma vez, use **Inspecionar Pasta de Trabalho** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de uso

**Enviar estatísticas anônimas de uso** permite compartilhar dados sobre como o JeffCrunch é usado. Anônimo significa que os dados não dizem nada sobre quem você é. Quando esta opção está ativada, o JeffCrunch envia esses dados uma vez por dia e no dia em que você atualiza: que é o JeffCrunch, sua versão, seu sistema operacional e idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou. Se o JeffCrunch fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia seus arquivos, os nomes deles, nem nada do que há neles, nem o que você digitou. Se esta opção estiver desativada, nenhum relatório de falha é enviado.

A escolha vale para todos os aplicativos do JeffOffice: altere-a aqui e ela muda em cada um deles. O JeffCrunch pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele informa o que é enviado, para onde vai e por quanto tempo guardamos os dados. Mantemos as estatísticas por 24 meses.
- **Número de instalação** aparece abaixo da caixa de seleção. É um número aleatório criado para a sua cópia do JeffOffice, o mesmo em todos os aplicativos do JeffOffice. Ele não diz nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número ainda tiver sido criado, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Nada é então contado ou enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Segurança

A guia **Segurança** define o que o JeffCrunch faz com as coisas para as quais um arquivo pode apontar: arquivos vinculados, filmes e sons vinculados e arquivos da internet. As macros ficam guardadas no arquivo e nunca são executadas, e um link que iniciaria um programa é sempre recusado. Veja [Segurança](security).

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
