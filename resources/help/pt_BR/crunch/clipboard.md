# Recortar, copiar e colar
<!-- keywords: recortar, copiar, colar, área de transferência, mover, duplicar, colar especial, valores, fórmulas, formatos, transpor, ignorar em branco, somar, adicionar, subtrair, multiplicar, dividir, operação, mescladas, texto, tabulação, html, página da Web, outro programa, Esc, borda tracejada, formigas marchando, borda -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um lugar que o computador reserva para guardar uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a partir da célula selecionada. **Recortar** (**{key:Cut}**) funciona como copiar, mas, quando você cola, as células são movidas e o local antigo fica vazio.

Uma borda tracejada em movimento mostra as células que foram copiadas ou recortadas. Pressione **Esc** para remover a borda; as células deixam então de estar prontas para colar. As células recortadas também são liberadas quando você faz qualquer outra alteração antes de colá-las (digitar, inserir, desfazer): recorte-as novamente se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo é levado junto: os valores, as fórmulas, os formatos e as células mescladas. Uma fórmula continua apontando para os mesmos locais *em relação à posição dela*. Exemplo: copie **=A1*2** de B1 e cole em B5, e ela se torna **=A5*2**. Uma parte com cifrão, como **$A$1**, permanece como está. Se você selecionar um bloco cujo tamanho seja um múltiplo exato do que você copiou, a cópia é repetida para preenchê-lo: copie uma célula, selecione vinte células, cole, e todas as vinte recebem o conteúdo.

Um número colado que seja largo demais para uma coluna que ainda tem a largura original faz a coluna ficar mais larga.

## Recortar move as células

Quando você recorta células e as cola, as células são realmente movidas. Toda fórmula que apontava para elas passa a apontar para elas no novo local. Exemplo: B1 contém **=A1+A2**. Recorte A1:A2 e cole em E4, e B1 se torna **=E4+E5**. O que havia nas células sobre as quais você colou é perdido (**Desfazer** traz de volta), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que ela lia não existe mais. Se as células recortadas forem apenas parte de uma célula mesclada, ou se você colar sobre apenas parte de uma, nada é movido: selecione a célula mesclada inteira. Quando você recorta células para outra planilha, as fórmulas continuam lendo as mesmas células que liam antes (elas indicam qual é a planilha). A área de transferência é esvaziada depois de recortar e colar.

## Colar Especial

**Ctrl+Alt+V**, ou a seta abaixo de **Colar** na guia Página Inicial, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que ela mostra.
- Uma **operação**: **Adicionar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células contêm 10 e 20, você copiou 5 e escolheu *Multiplicar*: elas passam a ser 50 e 100.
- **Ignorar em branco**: as células vazias da cópia deixam como estavam as células em que caem.
- **Transpor**: as linhas se tornam colunas e as colunas se tornam linhas.

## Entre programas

Quando você copia, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por linha de texto) e como tabela para páginas da Web, para que outros programas possam usá-las. Quando você cola algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página da Web ou de outra planilha, cada parte vai para sua própria célula e é lida como se você a tivesse digitado: números, datas e fórmulas que começam com sinal de igual. Copiar de outra pasta de trabalho aberta no JeffCrunch traz valores, fórmulas e formatos de número, mas não a demais formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula de matriz**, a linha de nomes de coluna de uma **tabela**, células bloqueadas de uma **planilha protegida**, uma área além do fim da planilha ou mais de 100.000 células. Uma **fórmula de matriz** inteira pode ser recortada e movida (selecione todas as células dela); parte de uma, uma tabela de dados ou a linha de nomes de uma tabela não podem.
