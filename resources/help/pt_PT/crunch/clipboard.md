# Cortar, copiar e colar
<!-- keywords: cortar, copiar, colar, área de transferência, mover, duplicar, colar especial, valores, fórmulas, formatos, transpor, ignorar células em branco, somar, adicionar, subtrair, multiplicar, dividir, operação, células unidas, texto, tabulação, html, página web, outro programa, Esc, Escape, limite tracejado, contorno -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um local onde o computador guarda uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a partir da célula selecionada. **Cortar** (**{key:Cut}**) é semelhante a copiar, mas, quando cola, as células são movidas e o local antigo fica vazio.

Um limite tracejado em movimento mostra as células que foram copiadas ou cortadas. Prima **Esc** para retirar o limite; as células deixam então de estar prontas para colar. As células cortadas também são libertadas quando faz qualquer outra alteração antes de as colar (escrever, inserir, anular): corte-as novamente se ainda as quiser mover.

## Colar no JeffCrunch

Tudo é incluído: os valores, as fórmulas, os formatos e as células unidas. Uma fórmula continua a apontar para os mesmos locais *relativamente à posição onde está*. Exemplo: copie **=A1*2** de B1 e cole-a em B5, e passa a **=A5*2**. Uma parte com cifrão, como **$A$1**, mantém-se inalterada. Se selecionar um bloco cujo tamanho seja um múltiplo inteiro do que copiou, a cópia é repetida para o preencher: copie uma célula, selecione vinte células, cole, e as vinte recebem-na.

Um número colado que seja demasiado largo para uma coluna que ainda tem a largura inicial alarga a coluna.

## Cortar move as células

Quando corta células e as cola, as células são realmente movidas. Todas as fórmulas que apontavam para elas passam a apontar para a nova posição. Exemplo: B1 contém **=A1+A2**. Corte A1:A2 e cole em E4, e B1 passa a **=E4+E5**. O que estava nas células sobre as quais colou desaparece (**Anular** repõe-no), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que lia já não existe. Se as células que cortou forem apenas parte de uma célula unida, ou se colar apenas sobre parte de uma, nada é movido: selecione a célula unida inteira. Quando corta células para outra folha, as fórmulas continuam a ler as células que liam antes (indicando a folha). A área de transferência é esvaziada depois de cortar e colar.

## Colar Especial

**Ctrl+Alt+V**, ou a seta por baixo de **Colar** no separador Base, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que mostra.
- Uma **operação**: **Adicionar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números que copiou são combinados com os números que já estão nas células. Exemplo: as células contêm 10 e 20, copiou 5 e escolheu *Multiplicar*: passam a 50 e 100.
- **Ignorar células em branco**: as células vazias da cópia deixam inalteradas as células onde são coladas.
- **Transpor**: as linhas passam a colunas e as colunas passam a linhas.

## Entre programas

Quando copia, o JeffCrunch coloca também as células na área de transferência como texto (células separadas por tabulações, uma linha por linha de texto) e como tabela para páginas web, para que outros programas as possam utilizar. Quando cola algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página web ou de outra folha de cálculo, cada elemento vai para a sua própria célula e é lido como se o tivesse escrito: números, datas e fórmulas que começam por um sinal de igual. Copiar de outro livro aberto no JeffCrunch traz valores, fórmulas e formatos de número, mas não a restante formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem abrangeria parte de uma **fórmula de matriz**, a linha de nomes de colunas de uma **tabela**, células bloqueadas de uma **folha protegida**, uma área para lá do fim da folha ou mais de 100 000 células. Uma **fórmula de matriz** inteira pode ser cortada e movida (selecione todas as suas células); parte de uma, uma tabela de dados ou a linha de nomes de uma tabela não podem.
