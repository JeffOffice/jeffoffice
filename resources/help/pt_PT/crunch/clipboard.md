# Cortar, copiar e colar

<!-- keywords: cortar, copiar, colar, área de transferência, mover, duplicar, especial, valores, fórmulas, formatos, transpor, ignorar células em branco, adicionar, subtrair, multiplicar, dividir, operação, unidas, texto, separador, html, página web, outro programa, Esc, marcha de formigas, limite, larguras de coluna, notas, comentários, formatos de número, tudo exceto limites, imagem, bitmap, copiar como imagem, imagem impressa, como impresso, colar ligação, hiperligação, importação de texto, dividir texto, separador, definir colagem predefinida, colagem predefinida, largura fixa -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a começar na célula selecionada. **Cortar** (**{key:Cut}**) funciona como copiar, mas, ao colar, as células são movidas e o local original fica vazio.

Um limite tracejado em movimento mostra as células que foram copiadas ou cortadas. Prima **Esc** para retirar o limite; as células deixam então de estar prontas para colar. As células cortadas também são libertadas se fizer qualquer outra alteração antes de as colar (escrever, inserir, anular): corte-as novamente se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Vem tudo: os valores, as fórmulas, os formatos e as células unidas. Uma fórmula continua a apontar para os mesmos locais *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole-a em B5, e passa a ser **=A5*2**. Uma parte com um sinal de dólar, como **$A$1**, mantém-se como está. Se selecionar um bloco que seja um múltiplo exato do tamanho do que copiou, a cópia é repetida para o preencher: copie uma célula, selecione vinte células, cole, e todas as vinte ficam com o conteúdo.

Um número colado que seja demasiado largo para uma coluna que ainda tenha a largura inicial alarga a coluna.

## Cortar move células

Quando corta células e as cola, as células movem-se efetivamente. Todas as fórmulas que apontavam para elas passam a apontar para a nova posição. Exemplo: B1 tem **=A1+A2**. Corte A1:A2 e cole em E4, e B1 passa a ser **=E4+E5**. O que estava nas células sobre as quais colou desaparece (**Anular** repõe), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que lia já não existe. Se as células cortadas forem apenas parte de uma célula unida, ou se colar sobre apenas parte de uma, nada é movido: selecione a célula unida inteira. Ao cortar células para outra folha, as fórmulas continuam a ler as células que liam antes (indicam a folha). A área de transferência fica vazia depois de um corte e colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta abaixo de **Colar** no separador Base, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que mostra.
- **Tudo Exceto Limites**: tudo, mas as células mantêm os limites que tinham.
- **Fórmulas e Formatos de Número**, ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são apresentados, como percentagem ou data, mas não as cores, os tipos de letra ou os limites. As células mantêm o aspeto que tinham.
- **Larguras de Coluna**: as colunas onde colar ficam com as larguras das colunas copiadas.
- **Comentários e Notas**: as notas das células copiadas são colocadas nas células onde colar. Nenhum valor de célula muda.
- Uma **operação**: **Adicionar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células têm 10 e 20, copiou 5, e escolheu *Multiplicar*: passam a ser 50 e 100.
- **Ignorar células em branco**: as células vazias na cópia deixam as células de destino como estavam.
- **Transpor**: as linhas passam a colunas e as colunas passam a linhas.

## Colar Ligação, Colar como Hiperligação e Colar Importação de Texto

Há mais três opções na seta abaixo de **Colar**. Funcionam com células copiadas no mesmo livro, exceto a última.

- **Colar Ligação** coloca uma fórmula em cada célula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar Ligação**: D4 passa a ser **=Folha1!$A$1**. Quando A1 mudar, D4 muda com ela. Uma célula copiada que esteja vazia mostra 0.
- **Colar como Hiperligação** coloca em cada célula as mesmas palavras que a célula copiada mostra, e transforma cada uma numa ligação para a célula copiada. Mantenha **Ctrl** premido e clique numa delas para ir a essa célula. Funciona com até 2000 células de cada vez.
- **Colar Importação de Texto** serve para texto de outro programa. Abre a mesma janela que a abertura de um ficheiro de texto: indica o que separa as partes (uma vírgula, um tabulador, um espaço, um ponto e vírgula, uma barra vertical ou outro carácter), o que delimita uma parte, a primeira linha a utilizar e a forma de ler cada coluna (como escrito, como texto, como data ou omitida). Algum texto alinha as partes em colunas da mesma largura, com espaços entre elas. Marque **As colunas têm uma largura fixa** e indique onde começa cada coluna: cada linha é cortada nesses pontos, e os espaços em redor de cada parte são retirados. Escolha *texto* para uma coluna de códigos como 007, para que os zeros se mantenham. As partes são colocadas nas células, a partir da célula selecionada, num único passo que pode ser anulado.

## Definir Colagem Predefinida

**Definir Colagem Predefinida** é a última opção na seta abaixo de **Colar**. Decide o que o botão **Colar** e **{key:Paste}** colocam quando copiou células: tudo (inicialmente), ou apenas os valores, apenas as fórmulas, apenas a formatação, e assim por diante. Exemplo: escolha *Valores*, e uma fórmula copiada fica como o número que mostra. A escolha fica guardada para a próxima vez. Não altera o **Colar Especial** nem as opções do menu, e o texto de outros programas é colado tal como está. Escolha *Tudo* para voltar ao normal.

## O menu Colar e as imagens

A seta abaixo de **Colar** apresenta as opções habituais e também **Sem Limites**, **Fórmulas e Formatação de Números**, **Valores e Formatação de Números**, **Manter Larguras de Coluna de Origem** e **Comentários e Notas**. São as mesmas opções do Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como Imagem** e **Copiar como Bitmap**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada como as vê no ecrã, com as linhas de grelha. Um bitmap tem o tamanho do ecrã. Uma imagem tem o dobro dos pontos, por isso mantém-se nítida quando é ampliada. **Copiar como Imagem Impressa** desenha as células como o papel as mostraria: as linhas de grelha só aparecem se a configuração da página as imprimir. Cole-a no JeffWrite ou no JeffDeck como imagem. Se as células não couberem numa só imagem, selecione menos.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabuladores, uma linha por linha) e como tabela para páginas web, para que outros programas as possam receber. Quando cola algo que não veio do JeffCrunch, texto com tabuladores, ou uma tabela de uma página web ou de outra folha de cálculo, cada parte vai para a sua própria célula e é lida como se a tivesse escrito: números, datas e fórmulas que começam com um sinal de igual. Copiar de outro livro aberto no JeffCrunch traz valores, fórmulas e formatos de número, mas não a restante formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula matricial**, por cima da linha de nomes de colunas de uma **tabela**, em células bloqueadas de uma **folha protegida**, para além do fim da folha, ou em mais de 100 000 células. Uma **fórmula matricial** completa pode ser cortada e movida (selecione todas as suas células); parte de uma, uma tabela de dados ou a linha de nomes de uma tabela não podem.
