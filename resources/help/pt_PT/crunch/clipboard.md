# Cortar, copiar e colar

<!-- keywords: cortar, copiar, colar, área de transferência, mover, duplicar, colar especial, valores, fórmulas, formatos, transpor, ignorar células em branco, somar, subtrair, multiplicar, dividir, operação, unidas, texto, tabulação, html, página web, outro programa, Esc, formiguinhas, limite, larguras das colunas, notas, comentários, formatos de número, tudo exceto limites, imagem, bitmap, copiar como imagem, imagem impressa, como impresso, colar ligação, hiperligação, importação de texto, dividir texto, separador -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a partir da célula selecionada. **Cortar** (**{key:Cut}**) é como copiar, mas, ao colar, as células são movidas e o local original fica vazio.

Um limite tracejado em movimento mostra as células que foram copiadas ou cortadas. Prima **Esc** para retirar o limite; as células deixam então de estar prontas para colar. As células cortadas também deixam de estar disponíveis se fizer outra alteração antes de as colar (escrever, inserir, anular): corte-as de novo se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo é transferido: os valores, as fórmulas, os formatos e as células unidas. Uma fórmula continua a apontar para os mesmos locais *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e passa a ser **=A5*2**. Uma parte com um sinal de dólar, como **$A$1**, mantém-se tal como está. Se selecionar um bloco que seja um número inteiro de vezes o tamanho do que copiou, a cópia é repetida para o preencher: copie uma célula, selecione vinte células, cole, e todas as vinte ficam com o conteúdo.

Um número colado que seja demasiado largo para uma coluna que ainda tem a sua primeira largura aumenta a coluna.

## Cortar move células

Quando corta células e as cola, as células são realmente movidas. Todas as fórmulas que apontavam para elas passam a apontar para a sua nova posição. Exemplo: B1 tem **=A1+A2**. Corte A1:A2 e cole em E4, e B1 passa a ser **=E4+E5**. O que estava nas células sobre as quais colou perde-se (**Anular** repõe-no), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que lia já não existe. Se as células cortadas forem apenas parte de uma célula unida, ou se colar sobre apenas parte de uma, nada é movido: selecione a célula unida inteira. Quando corta células para outra folha, as fórmulas continuam a ler as células que liam antes (indicam a folha). A área de transferência fica vazia depois de um corte e colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta por baixo de **Colar** no separador Base, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que mostra.
- **Tudo Exceto Limites**: tudo, mas as células mantêm os limites que tinham.
- **Fórmulas e Formatos de Número**, ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são apresentados, como percentagem ou data, mas não as cores, os tipos de letra ou os limites. As células mantêm o aspeto que tinham.
- **Larguras das Colunas**: as colunas onde colar ficam com as larguras das colunas copiadas.
- **Comentários e Notas**: as notas das células copiadas são colocadas nas células onde colar. Nenhum valor de célula é alterado.
- Uma **operação**: **Somar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células têm 10 e 20, copiou 5, e escolheu *Multiplicar*: passam a ter 50 e 100.
- **Ignorar células em branco**: as células vazias da cópia deixam as células de destino como estavam.
- **Transpor**: as linhas passam a colunas e as colunas passam a linhas.

## Colar Ligação, Colar como Hiperligação e Colar Importação de Texto

Há mais três opções na seta por baixo de **Colar**. Funcionam com células copiadas no mesmo livro, exceto a última.

- **Colar Ligação** coloca em cada célula uma fórmula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar Ligação**: D4 passa a ser **=Folha1!$A$1**. Quando A1 mudar, D4 muda com ela. Uma célula copiada que esteja vazia mostra 0.
- **Colar como Hiperligação** coloca em cada célula as mesmas palavras que a célula copiada mostra, e transforma cada uma numa ligação para a célula copiada. Mantenha **Ctrl** premido e clique numa para ir para essa célula. Funciona em até 2000 células de cada vez.
- **Colar Importação de Texto** serve para texto vindo de outro programa. Abre a mesma janela que a abertura de um ficheiro de texto: indica o que separa os elementos (uma vírgula, um tabulador, um espaço, um ponto e vírgula, uma barra vertical ou outro carácter), o que delimita um elemento, a primeira linha a utilizar e a forma de ler cada coluna (tal como escrita, como texto, como data, ou omitida). Escolha *texto* para uma coluna de códigos como 007, para que os zeros se mantenham. Os elementos são colocados nas células, a partir da célula selecionada, como um único passo que pode anular.

## O menu Colar e as imagens

A seta por baixo de **Colar** apresenta as opções habituais e também **Sem Limites**, **Formatar Fórmulas e Números**, **Formatar Valores e Números**, **Manter Larguras das Colunas de Origem** e **Comentários e Notas**. São as mesmas opções de Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como Imagem** e **Copiar como Bitmap**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada tal como as vê, com as linhas de grelha. Um bitmap tem o tamanho do ecrã. Uma imagem tem o dobro dos pontos, por isso mantém-se nítida quando é ampliada. **Copiar como Imagem Impressa** desenha as células tal como o papel as mostraria: as linhas de grelha só aparecem se a configuração da página as imprimir. Cole-a no JeffWrite ou no JeffDeck como imagem. Se as células não couberem numa só imagem, selecione menos.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por linha) e como tabela para páginas web, para que outros programas as possam utilizar. Quando cola algo que não veio do JeffCrunch, texto com tabulações, ou uma tabela de uma página web ou de outra folha de cálculo, cada elemento vai para a sua própria célula e é lido como se o tivesse escrito: números, datas e fórmulas que começam com um sinal de igual. Copiar de outro livro aberto no JeffCrunch transfere valores, fórmulas e formatos de número, mas não a restante formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula matricial**, por cima da linha de nomes de colunas de uma **tabela**, em células bloqueadas de uma **folha protegida**, para além do fim da folha, ou em mais de 100 000 células. Uma **fórmula matricial** completa pode ser cortada e movida (selecione todas as suas células); parte de uma, uma tabela de dados ou a linha de nomes de uma tabela não podem.
