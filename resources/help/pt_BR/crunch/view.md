# Alterar a exibição

<!-- keywords: estrutura de tópicos, agrupar, desagrupar, recolher, expandir, detalhe, nível, exibição, zoom, da direita para a esquerda, árabe, hebraico, espelhado, congelar, congelar painéis, descongelar, linhas de grade, títulos, barra de fórmulas, recolher a faixa de opções, tela cheia, barra de status, mínimo, máximo, mostrar fórmulas, guias de planilha, planilhas ocultas, cabeçalhos -->

A guia **Exibir** muda a aparência da planilha na sua tela. Ela nunca altera a pasta de trabalho.

## Modos de exibição

O grupo **Modos de Exibição** da guia Exibir (e os três botões no canto inferior direito da janela) alteram o que a planilha mostra. **Normal** mostra as células como elas são. **Layout da Página** mostra as páginas como serão impressas, com o cabeçalho e o rodapé. **Visualização de Quebras de Página** sombreia o que não será impresso, numera as páginas e permite arrastar as linhas em que as páginas começam. Veja [Configurar as páginas](page-layout) para os dois últimos. Cada planilha lembra o seu modo de exibição, e a pasta de trabalho o salva.

## Zoom

Use o controle deslizante no canto inferior direito, o botão **Zoom** na guia Exibir ou mantenha **Ctrl** pressionada e gire a roda do mouse. Você pode aplicar zoom de 10% a 400%. **100%** volta ao tamanho normal.

## Mostrar e ocultar

No grupo **Mostrar**, ative ou desative as **Linhas de Grade** (as linhas claras entre as células), os **Títulos** (as letras das colunas e os números das linhas) e a **Barra de Fórmulas**. Uma pasta de trabalho pode pedir que as linhas de grade ou os títulos fiquem desativados; essa escolha é sua para alterar.

Há mais duas opções no mesmo grupo. **Recolher a Faixa de Opções** (**Ctrl+F1**) recolhe a faixa de opções para que ela mostre apenas os nomes das guias, deixando mais espaço para as células. Clique nela novamente, ou clique em uma guia, para trazê-la de volta. **Tela Cheia** (**Ctrl+Shift+F1**) faz a janela ocupar a tela inteira. Clique novamente para voltar.

## Os números na barra de status

Quando você seleciona duas ou mais células, a barra de status mostra números sobre elas. **Clique com o botão direito** nos números para escolher quais aparecem: **Média**, **Contagem** (as células que contêm algo), **Contagem Numérica** (somente as células que contêm números), **Mínimo** (o menor número), **Máximo** (o maior número) e **Soma**. Uma marca de verificação indica as que estão ativadas. Pelo menos uma permanece ativada. O JeffCrunch lembra a sua escolha.

## Congelar linhas e colunas

As linhas e colunas **congeladas** permanecem no lugar enquanto o restante da planilha rola, para que uma linha de cabeçalho esteja sempre visível. Na guia **Exibir**, clique em **Congelar Painéis**:

- **Congelar Painéis** congela as linhas acima da célula selecionada e as colunas à esquerda dela, a partir da primeira linha e da primeira coluna que você vê. As linhas e colunas que você rolou para fora da tela permanecem fora dela até você descongelar.
- **Congelar Linha Superior** congela a primeira linha visível.
- **Congelar Primeira Coluna** congela a primeira coluna visível.
- **Descongelar Painéis** desfaz o congelamento, e a planilha permanece onde está.

As linhas e colunas congeladas são salvas com a pasta de trabalho, então são mantidas novamente quando ela é aberta.

Uma pasta de trabalho salva com painéis congelados é aberta com eles.

## Planilhas

As guias na parte inferior são as planilhas da pasta de trabalho. Clique em uma guia para mostrar essa planilha. Cada planilha lembra onde você estava nela. Planilhas ocultas não são mostradas. Uma linha colorida sob a guia é a cor da guia da planilha. Para adicionar, renomear, mover, copiar, ocultar, excluir ou colorir uma planilha, veja [Trabalhar com planilhas](sheets).

## Planilhas que são lidas da direita para a esquerda

Uma planilha feita para um idioma como árabe ou hebraico é lida da direita para a esquerda, e o JeffCrunch a mostra dessa forma: a coluna A fica na borda direita, os números das linhas ficam à direita, e a barra de rolagem horizontal funciona no sentido contrário. As teclas de seta seguem a planilha, então a seta para a esquerda vai para a próxima coluna. O texto em uma célula começa do lado para o qual o seu idioma é lido. A pasta de trabalho indica em que sentido a planilha é lida, e você pode alterar isso: na guia **Layout da Página**, clique em **Planilha da Direita para a Esquerda**. Clique novamente para desativar. A escolha é salva com a planilha.

## Agrupar linhas e colunas (estrutura de tópicos)

Na guia **Dados**, **Agrupar Linhas** adiciona um nível às linhas selecionadas, e **Desagrupar Linhas** remove um nível; **Agrupar Colunas** e **Desagrupar Colunas** fazem o mesmo com colunas. As linhas podem ser agrupadas dentro de outros grupos, até sete níveis. Ao lado das linhas (e acima das colunas), a planilha mostra um colchete para cada grupo e, na linha que totaliza o grupo (a linha abaixo das linhas, ou à direita das colunas, a menos que o seu arquivo as coloque acima ou à esquerda), um botão: **menos** oculta as linhas do grupo, e **mais** as mostra novamente. Os botões numerados no canto mostram a estrutura de tópicos até determinado nível: **1** mostra apenas as linhas que não estão em nenhum grupo, **2** também as linhas do primeiro nível, e assim por diante. Um grupo dentro de um grupo oculto continua oculto quando o grupo externo é mostrado, até que o próprio botão seja clicado. Cada uma dessas ações é uma etapa de Desfazer, e as linhas ocultas e os grupos são salvos com a pasta de trabalho. Desagrupar um grupo oculto mostra as suas linhas.

**Exibir > Símbolos de Estrutura de Tópicos** ativa e desativa os colchetes e os botões.

## Mostrar fórmulas

Na guia **Fórmulas**, **Mostrar Fórmulas** (Ctrl+`; em um Mac, a tecla Control, não a tecla Command) mostra a fórmula em cada célula que tem uma, no lugar do seu valor. As colunas que contêm fórmulas são alargadas, até um limite, para que as fórmulas apareçam por inteiro, e voltam às larguras originais quando você desativa a opção; as larguras no seu arquivo não são alteradas.
