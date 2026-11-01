# Alterar o modo de exibição
<!-- keywords: estrutura de tópicos, agrupar, desagrupar, recolher, expandir, detalhe, nível, exibir, modo de exibição, zoom, da direita para a esquerda, árabe, hebraico, espelhado, congelar, congelar painéis, descongelar, linhas de grade, títulos de linha e coluna, cabeçalhos, barra de fórmulas, mostrar fórmulas, guias de planilha, abas, planilhas ocultas -->

A guia **Exibir** altera a aparência da planilha na tela. Ela nunca altera a pasta de trabalho.

## Modos de exibição

O grupo **Modos de Exibição** da guia Exibir (e os três botões no canto inferior direito da janela) alteram o que a planilha mostra. **Normal** mostra as células como elas são. **Layout da Página** mostra as páginas como serão impressas, com o cabeçalho e o rodapé. **Quebras de Página** sombreia o que não será impresso, numera as páginas e permite arrastar as linhas onde as páginas começam. Consulte [Configurar as páginas](page-layout) para saber mais sobre os dois últimos. Cada planilha lembra o seu modo de exibição, e a pasta de trabalho o salva.

## Zoom

Use o controle deslizante no canto inferior direito, o botão **Zoom** da guia Exibir ou mantenha **Ctrl** pressionada e gire a roda do mouse. Você pode aplicar zoom de 10% a 400%. **100%** volta ao tamanho normal.

## Mostrar e ocultar

No grupo **Mostrar**, ative ou desative as **Linhas de Grade** (as linhas claras entre as células), os **Títulos** (as letras das colunas e os números das linhas) e a **Barra de Fórmulas**. Uma pasta de trabalho pode pedir que as linhas de grade ou os títulos fiquem desativados; você pode alterar essa opção.

## Congelar linhas e colunas

As linhas e colunas **congeladas** permanecem no lugar enquanto o restante da planilha rola, de modo que uma linha de cabeçalho fica sempre visível. Na guia **Exibir**, clique em **Congelar Painéis**:

- **Congelar Painéis** congela as linhas acima da célula selecionada e as colunas à esquerda dela, a partir da primeira linha e coluna visíveis. As linhas e colunas que você rolou para fora da tela continuam ocultas até você descongelar.
- **Congelar Linha Superior** congela a primeira linha visível.
- **Congelar Primeira Coluna** congela a primeira coluna visível.
- **Descongelar Painéis** desfaz o congelamento, e a planilha permanece onde está.

As linhas e colunas congeladas são salvas com a pasta de trabalho e voltam a ficar congeladas quando ela é aberta.

Uma pasta de trabalho salva com painéis congelados é aberta com eles.

## Planilhas

As guias na parte inferior são as planilhas da pasta de trabalho. Clique em uma guia para mostrar a planilha correspondente. Cada planilha lembra onde você estava nela. As planilhas ocultas não são mostradas. Uma linha colorida sob uma guia indica a cor da guia da planilha. Para adicionar, renomear, mover, copiar, ocultar, excluir ou colorir uma planilha, consulte [Trabalhar com planilhas](sheets).

## Planilhas lidas da direita para a esquerda

Uma planilha criada para um idioma como árabe ou hebraico é lida da direita para a esquerda, e o JeffCrunch a mostra assim: a coluna A fica na borda direita, os números das linhas ficam à direita e a barra de rolagem horizontal funciona no sentido inverso. As teclas de direção seguem a planilha, portanto a seta para a esquerda vai para a próxima coluna. O texto de uma célula começa pelo lado de onde o idioma dele é lido. A pasta de trabalho define a direção de leitura de cada planilha; o JeffCrunch ainda não tem um comando para alterá-la.

## Agrupar linhas e colunas (estrutura de tópicos)

Na guia **Dados**, **Agrupar Linhas** aumenta em um o nível das linhas selecionadas, e **Desagrupar Linhas** diminui em um; **Agrupar Colunas** e **Desagrupar Colunas** fazem o mesmo com as colunas. É possível agrupar linhas dentro de grupos, até sete níveis. Ao lado das linhas (e acima das colunas), a planilha mostra um colchete para cada grupo e, na linha que resume o grupo (a linha abaixo das linhas ou à direita das colunas, a menos que o seu arquivo as coloque acima ou à esquerda), um botão: **menos** oculta as linhas do grupo e **mais** as mostra novamente. Os botões numerados no canto mostram a estrutura de tópicos até um nível: **1** mostra somente as linhas que não pertencem a nenhum grupo, **2** mostra também as linhas do primeiro nível, e assim por diante. Um grupo dentro de um grupo oculto permanece oculto quando o grupo externo é mostrado, até que o botão dele seja clicado. Cada uma dessas ações é uma etapa de Desfazer, e as linhas ocultas e os grupos são salvos com a pasta de trabalho. Desagrupar um grupo oculto mostra as linhas dele.

**Exibir > Símbolos de Estrutura de Tópicos** desativa e ativa os colchetes e botões.

## Mostrar fórmulas

Na guia **Fórmulas**, **Mostrar Fórmulas** (Ctrl+`; no Mac, a tecla Control, não a tecla Command) mostra a fórmula em todas as células que têm uma, no lugar do valor. As colunas que contêm fórmulas ficam mais largas, até um limite, para que as fórmulas apareçam inteiras, e voltam à largura original quando você desativa a opção; as larguras no seu arquivo não são alteradas.
