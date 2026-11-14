# Alterar as configurações do JeffCrunch
<!-- keywords: recuperação, recuperar trabalho não salvo, recuperação automática, revisão de texto, edição, preenchimento automático, alça de preenchimento, arrastar e soltar, casas decimais, opções, configurações, preferências, estatísticas de uso, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, excluir estatísticas, atualizações, sobre, editar na célula, cor das linhas de grade, zoom padrão, largura padrão, privacidade, informações pessoais, personalizar teclado, personalizar a faixa de opções, personalizar a barra de ferramentas rápida, barra de ferramentas rápida, adicionar comando à barra de ferramentas rápida, ocultar guia, ocultar grupo, adicionar guia, adicionar grupo, renomear guia, redefinir a faixa de opções, alterar teclas de atalho, reatribuir teclas, redefinir teclas -->

Suas configurações ficam na janela **Opções do JeffCrunch**. Clique em **Arquivo > Opções** para abri-la. Clique em **OK** para manter as alterações ou em **Cancelar** para descartá-las.

## Edição

A página **Edição** altera como a digitação nas células funciona.

- **Após pressionar Enter, mover a seleção** define para onde vai a seleção quando você termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lugar nenhum.
- **Oferecer para concluir um texto a partir das células acima e abaixo** é chamado de Preenchimento automático. Quando você digita as primeiras letras de um texto e apenas um texto dessa coluna começa com elas, o JeffCrunch mostra o restante, selecionado. Pressione **Enter** para mantê-lo. Continue digitando para ignorá-lo, ou pressione **Backspace** para removê-lo. Ele considera apenas o intervalo de células ao lado daquela em que você está digitando, e somente textos, não números.
- **Usar a alça de preenchimento e arrastar células para movê-las ou copiá-las** ativa o pequeno quadrado no canto da seleção e o arrastar pela borda da seleção. Desative se você move células sem querer. Copiar e Colar continuam funcionando.
- **Mostrar zero em células que têm valor zero (esta planilha)** ativa ou desativa os zeros na planilha que você está vendo. Quando está desativado, uma célula que contém 0 parece vazia, mas continua contendo o 0 e é somada como 0. A escolha é salva com a planilha.
- **Colocar um ponto decimal em números inteiros digitados** é chamado de casas decimais automáticas. Defina **Casas** como 2, e digitar 1234 insere 12,34. Defina como -2, e digitar 5 insere 500. Um número digitado com vírgula decimal é inserido como digitado.

- **Permitir edição diretamente nas células** vem ativado por padrão. Quando você desativa, uma entrada digitada ou F2 vai para a barra de fórmulas, e a célula mostra o resultado somente quando você conclui a entrada.

## Opções de salvamento

**Salvar informações de recuperação automática a cada** define com que frequência o JeffCrunch guarda uma cópia de uma pasta de trabalho com alterações que você ainda não salvou, de 1 a 120 minutos. A cópia é um arquivo .xlsx em uma pasta que o JeffCrunch mantém para essa finalidade, criada em segundo plano para que você possa continuar digitando. Ela é removida quando você salva ou fecha a pasta de trabalho. Se o JeffCrunch fechar inesperadamente, ele oferece as cópias na próxima vez que for iniciado. Você também pode clicar em **Arquivo > Abrir > Recuperar Trabalho Não Salvo**. Uma pasta de trabalho protegida com senha não é copiada, pois a cópia não ficaria protegida.

## Opções de revisão de texto

A página **Revisão de Texto** é compartilhada pelos aplicativos do JeffOffice. Ela define se a ortografia é verificada enquanto você digita em uma célula (desative **Verificar ortografia ao digitar** para eliminar os sublinhados vermelhos ondulados em todas as janelas abertas e nos outros aplicativos; **Ortografia** na guia Revisão continua funcionando), se palavras em letras maiúsculas e palavras com números são ignoradas na verificação ortográfica, se a Autocorreção corrige palavras enquanto você digita, se aspas retas viram aspas curvas, se travessões, frações e ordinais são alterados, e se um endereço da web vira um link. Consulte [Ortografia](notes-links) e [Digitar em células](edit-cells).

## Opções de exibição

- **Cor das linhas de grade** é a cor das linhas entre as células na sua tela. É uma escolha apenas para a sua tela, não salva na pasta de trabalho.
- **Zoom de uma nova pasta de trabalho** define o tamanho com que as planilhas de uma nova pasta de trabalho aparecem, de 10% a 400%.
- **Largura padrão de coluna (esta planilha)** é a largura de todas as colunas desta planilha que não têm largura própria. É o mesmo que **Largura Padrão** na guia Página Inicial.

## Personalizar

A guia **Personalizar** tem três páginas: **Faixa de Opções**, **Barra de Ferramentas Rápida** e **Teclado**. A página **Teclado** é onde você escolhe as teclas de atalho dos comandos do JeffCrunch. Consulte "Escolher suas próprias teclas de atalho" no tópico sobre teclado.

### Personalizar a faixa de opções

A página **Faixa de Opções** lista as guias da faixa, cada uma com seus grupos.

- **Ocultar ou mostrar uma guia ou um grupo.** Desmarque a caixa ao lado para ocultá-lo e marque a caixa para mostrá-lo novamente. Um grupo oculto não ocupa espaço, e seus comandos continuam funcionando pelas teclas de atalho. **Página Inicial** não tem caixa, pois sempre fica visível, e o botão **Arquivo** também não. Uma guia que aparece somente quando algo está selecionado, como uma imagem ou uma tabela, fica no fim da lista e não tem caixa.
- **Criar sua própria guia ou grupo.** Digite um nome na caixa **Nome** e clique em **Nova Guia**. Para criar um grupo, escolha uma guia e clique em **Novo Grupo**. Um grupo próprio pode ir em qualquer guia, inclusive em uma guia que aparece somente quando algo está selecionado.
- **Colocar comandos no seu grupo.** Escolha seu grupo, encontre um comando na lista **Comandos** (digite na caixa **Pesquisar comandos** para reduzir a lista) e clique em **Adicionar Comando**. Qualquer comando que tenha nome pode entrar em um grupo. Um comando pode estar em um grupo apenas uma vez.
- **Renomear.** Escolha uma guia, grupo ou comando próprio, digite o novo nome e clique em **Renomear**. Um comando renomeado faz a mesma coisa, e o novo nome aparece apenas no botão. Renomeie um comando com nome vazio para restaurar o nome original dele.
- **Reordenar.** Escolha uma guia e clique em **Mover para Cima** ou **Mover para Baixo**. Você pode mover as guias da própria faixa, assim como as suas, mas não as guias que aparecem somente quando algo está selecionado. Seus grupos e seus comandos são movidos da mesma forma dentro da sua guia ou grupo. Os grupos da própria faixa mantêm a ordem.
- **Remover.** Escolha uma guia, grupo ou comando próprio e clique em **Remover**.

**Redefinir Um** restaura a guia escolhida: ela volta a aparecer, seus grupos voltam a aparecer, e os grupos que você adicionou a ela são removidos. Uma guia própria é removida. **Redefinir Tudo** restaura a faixa inteira, inclusive a ordem das guias.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffCrunch mostram a sua faixa, assim como qualquer janela que você abrir depois. O JeffCrunch guarda somente o que difere da faixa habitual, então uma guia ou grupo que uma versão posterior acrescentar aparece por conta própria. Suas guias e grupos recebem Dicas de Tecla (as letras mostradas ao pressionar **Alt**) que não coincidem com as demais. Você pode ter 20 guias próprias, 30 grupos próprios em uma guia e 60 comandos em um grupo.

### Personalizar a barra de ferramentas rápida

A página **Barra de Ferramentas Rápida** é para a pequena barra no canto superior esquerdo da janela. Sua lista à esquerda mostra o que está na barra agora, e a lista **Comandos** à direita tem todos os comandos que têm nome (digite na caixa **Pesquisar comandos** para reduzi-la, do mesmo modo que na página **Faixa de Opções**).

- **Adicionar um comando.** Escolha-o na lista **Comandos** e clique em **Adicionar Comando**. Ele vai para o fim da barra. Um comando que abre um menu mantém o menu na barra. Um comando pode estar na barra apenas uma vez.
- **Clique com o botão direito.** Você também pode clicar com o botão direito em qualquer comando da faixa e escolher **Adicionar à Barra de Ferramentas Rápida**, ou clicar com o botão direito em um botão da barra e escolher **Remover da Barra de Ferramentas Rápida**. O mesmo menu tem **Personalizar a Barra de Ferramentas Rápida** e **Personalizar a Faixa de Opções**, que abrem esta página.
- **Mover para abaixo da faixa.** Marque **Mostrar Barra de Ferramentas Rápida abaixo da faixa de opções** para colocar a barra em uma linha sob a faixa, em vez de no canto superior esquerdo. Os dois menus do botão direito também têm **Mostrar Barra de Ferramentas Rápida Abaixo da Faixa de Opções**. O JeffCrunch lembra da escolha.
- **Adicionar um separador.** Clique em **Adicionar Separador** para colocar uma linha fina no fim e, depois, mova-a para onde quiser.
- **Reordenar.** Escolha um comando ou separador na lista da esquerda e clique em **Mover para Cima** ou **Mover para Baixo**.
- **Remover.** Escolha-o e clique em **Remover**.

**Redefinir Um** restaura o comando escolhido: no lugar em que o JeffCrunch normalmente o coloca, ou fora da barra, quando o JeffCrunch normalmente não o põe ali. **Redefinir Tudo** restaura a barra que acompanha o JeffCrunch. Redefinir Tudo nesta página não altera a sua faixa, e Redefinir Tudo na página **Faixa de Opções** não altera a sua barra.

Nada muda até você clicar em **OK**. Depois disso, todas as janelas abertas do JeffCrunch mostram a sua barra, assim como qualquer janela que você abrir depois. A barra pode ter 100 comandos e separadores no total. Quando há mais botões do que cabem em um terço da largura da janela, os restantes ficam atrás de um botão **»** no fim da barra, para que o título da janela nunca saia da tela. O JeffCrunch mantém um comando na sua lista quando uma versão posterior não o tem mais, e volta a mostrá-lo se o comando reaparecer. Os botões recebem Dicas de Tecla: os números de 1 a 9 e depois duas letras, na ordem em que estão na barra. A barra fica sempre no canto superior esquerdo, ao lado do botão **Arquivo**.

## Opções de privacidade

- **Remover informações pessoais deste arquivo ao salvar** retira o autor, a pessoa que salvou o arquivo por último, a empresa, o gerente, e os nomes e horários das anotações e comentários, do arquivo que é gravado (uma pasta de trabalho, uma cópia e uma pasta de trabalho enviada por email). O conteúdo do arquivo permanece como está. As propriedades personalizadas do arquivo são mantidas como estão, e o documento aberto não é alterado. É a mesma opção em todos os aplicativos do JeffOffice: altere-a em um e ela muda em todos. Para fazer isso apenas uma vez, use **Inspecionar Pasta de Trabalho** em [Informações](info). A mesma caixa está na página **Informações**.

## Estatísticas de uso

**Enviar estatísticas anônimas de uso** permite compartilhar dados sobre como o JeffCrunch é usado. Anônimo significa que os dados não revelam quem você é. Quando isso está ativado, o JeffCrunch envia esses dados uma vez por dia e no dia em que você atualiza: que é o JeffCrunch, sua versão, seu sistema operacional e idioma, quantas vezes ele foi iniciado, com que frequência cada comando é usado e que tipo de arquivo você abriu ou salvou. Se o JeffCrunch fechar inesperadamente, ele também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia seus arquivos, seus nomes, nem qualquer conteúdo deles, nem o que você digitou. Se esta opção estiver desativada, nenhum relatório de falha é enviado.

A escolha vale para todos os aplicativos do JeffOffice: altere-a aqui e ela muda em cada um deles. O JeffCrunch pergunta sobre isso na primeira vez que é iniciado, em uma janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de seleção, abre nosso aviso de privacidade em jeffoffice.com/privacy. Ele explica o que é enviado, para onde vai e por quanto tempo guardamos os dados. Guardamos as estatísticas por 24 meses.
- **Número de instalação** é mostrado abaixo da caixa de seleção. É um número aleatório criado para a sua cópia do JeffOffice, igual em todos os aplicativos do JeffOffice. Ele não revela nada sobre você. Clique em **Copiar** para copiá-lo. Se as estatísticas estiverem desativadas, ou se nenhum número tiver sido criado ainda, aparece **nenhum**.
- Para excluir as estatísticas que já temos, envie seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais registros são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de iniciá-lo. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
