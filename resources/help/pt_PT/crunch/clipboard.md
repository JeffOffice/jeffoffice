# Cortar, copiar e colar

<!-- keywords: cortar, copiar, colar, área de transferência, mover, duplicar, especial, valores, fórmulas, formatos, transpor, ignorar células em branco, adicionar, subtrair, multiplicar, dividir, operação, unidas, texto, tabulação, html, página web, outro programa, Esc, contorno tracejado, limite, larguras das colunas, notas, comentários, formatos de número, tudo exceto limites, imagem, mapa de bits, copiar como imagem, imagem impressa, como impresso, colar ligação, hiperligação, importação de texto, texto delimitado, largar texto, vírgula, ponto e vírgula, csv, separador, definir colagem predefinida, colagem predefinida, largura fixa -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a começar na célula selecionada. **Cortar** (**{key:Cut}**) funciona como copiar, mas, ao colar, as células são movidas e o local antigo fica vazio.

Um contorno tracejado em movimento mostra as células que foram copiadas ou cortadas. Prima **Esc** para retirar o contorno; as células deixam então de estar prontas para colar. As células cortadas também deixam de estar disponíveis se fizer outra alteração antes de as colar (escrever, inserir, anular): corte-as de novo se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Vem tudo: os valores, as fórmulas, os formatos e as células unidas. Uma fórmula continua a apontar para os mesmos locais *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e passa a ser **=A5*2**. Uma parte com um cifrão, como **$A$1**, mantém-se como está. Se selecionar um bloco que seja um múltiplo exato do tamanho do que copiou, a cópia é repetida para o preencher: copie uma célula, selecione vinte células, cole, e todas as vinte ficam com o conteúdo.

Um número colado que seja demasiado largo para uma coluna que ainda tenha a largura inicial alarga a coluna.

## Cortar move células

Quando corta células e as cola, as células são mesmo movidas. Todas as fórmulas que apontavam para elas passam a apontar para a nova posição. Exemplo: B1 tem **=A1+A2**. Corte A1:A2 e cole em E4, e B1 passa a ser **=E4+E5**. O que estava nas células onde colou perde-se (**Anular** repõe), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que lia já não existe. Se as células cortadas forem apenas parte de uma célula unida, ou se colar sobre apenas parte de uma, nada é movido: selecione a célula unida inteira. Ao cortar células para outra folha, as fórmulas continuam a ler as células que liam antes (indicam a folha). A área de transferência fica vazia após um corte e colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta sob **Colar** no separador Base, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que apresenta.
- **Tudo Exceto Limites**: tudo, mas as células mantêm os limites que tinham.
- **Fórmulas e Formatos de Número**, ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são apresentados, como percentagem ou data, mas não as cores, os tipos de letra ou os limites. As células mantêm o aspeto que tinham.
- **Larguras das Colunas**: as colunas onde colar ficam com as larguras das colunas copiadas.
- **Comentários e Notas**: as notas das células copiadas são colocadas nas células onde colar. Nenhum valor de célula muda.
- Uma **operação**: **Adicionar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células têm 10 e 20, copiou 5, e escolheu *Multiplicar*: passam a ser 50 e 100.
- **Ignorar células em branco**: as células vazias na cópia deixam as células de destino como estavam.
- **Transpor**: as linhas passam a colunas e as colunas a linhas.

## Colar Ligação, Colar como Hiperligação e Colar Importação de Texto

Há mais três opções na seta sob **Colar**. Funcionam com células copiadas no mesmo livro, exceto a última.

- **Colar Ligação** coloca em cada célula uma fórmula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar Ligação**: D4 passa a ser **=Folha1!$A$1**. Quando A1 mudar, D4 muda com ele. Uma célula copiada que esteja vazia mostra 0.
- **Colar como Hiperligação** coloca em cada célula as mesmas palavras que a célula copiada mostra, e transforma cada uma numa ligação para a célula copiada. Mantenha **Ctrl** premido e clique numa delas para ir a essa célula. Funciona em até 2000 células de cada vez.
- **Colar Importação de Texto** serve para texto vindo de outro programa. Abre a mesma janela que ao abrir um ficheiro de texto: indica o que separa as partes (uma vírgula, um tabulador, um espaço, um ponto e vírgula, uma barra vertical ou outro carácter), o que delimita uma parte, a primeira linha a usar e a forma de ler cada coluna (como escrito, como texto, como data, ou não importar). Alguns textos alinham as partes em colunas de largura igual, com espaços entre elas. Marque **As colunas têm largura fixa** e indique onde começa cada coluna: cada linha é cortada nesses pontos, e os espaços à volta de cada parte são retirados. Escolha *texto* para uma coluna de códigos como 007, para que os zeros se mantenham. As partes são colocadas nas células, a começar na célula selecionada, como um único passo que pode anular.

## Texto com vírgulas, pontos e vírgulas ou colunas

Quando o texto que cola de outro programa é claramente uma tabela, o JeffCrunch abre a mesma janela que ao abrir um ficheiro de texto, para que possa verificar como o texto foi separado antes de qualquer coisa ser colocada nas células. Isto acontece apenas com um **Colar** simples (**{key:Paste}**), nunca com **Colar Especial** ou uma das suas opções. Acontece quando o texto tem pelo menos duas linhas e nenhum tabulador, e ou todas as linhas têm o mesmo número de partes, pelo menos três, separadas por vírgulas, pontos e vírgulas ou barras verticais, com algo em cada coluna (uma parte entre aspas pode ter uma vírgula); ou as partes se alinham em colunas de largura igual, com os mesmos intervalos de espaços em todas as linhas (pelo menos três linhas e pelo menos dois intervalos). Números com separadores de milhares, como 1,234, e linhas de código ou listas que terminam em vírgula ou ponto e vírgula não são considerados tabelas. A janela começa com a forma como o texto foi identificado. Se os números tiverem vírgulas e as partes estiverem separadas por pontos e vírgulas, a vírgula é o separador decimal. Clique em **OK** para colocar as partes, ou em **Cancelar** para não colocar nada.

Exemplo: copie estas linhas de um correio eletrónico.

    nome,qtd,preço
    Maçã,12,1.50
    Pera,3,0.25

Se as colar com uma terceira coluna, como um preço, a janela mostra três colunas, com **Vírgula** marcada. Uma frase numa só linha, linhas com número diferente de vírgulas, ou linhas com apenas duas partes são coladas como palavras, sem janela. O texto com tabulações vai diretamente para as células, como sempre aconteceu. Para abrir a janela com qualquer texto, escolha **Colar Importação de Texto** (acima).

O mesmo acontece quando arrasta texto de outro programa e o larga na grelha: é colocado na célula selecionada. Uma largada em qualquer outro ponto da janela não faz nada.

## Definir Colagem Predefinida

**Definir Colagem Predefinida** é a última opção na seta sob **Colar**. Decide o que o botão **Colar** e **{key:Paste}** colocam quando copiou células: tudo (inicialmente), ou apenas os valores, apenas as fórmulas, apenas a formatação, e assim por diante. Exemplo: escolha *Valores*, e uma fórmula copiada fica como o número que mostra. A escolha é mantida para a próxima vez. Não altera **Colar Especial** nem as opções do menu, e o texto de outros programas é colado tal como está. Escolha *Tudo* para voltar ao normal.

## O menu Colar e as imagens

A seta sob **Colar** lista as opções habituais e também **Sem Limites**, **Formatação de Números e Fórmulas**, **Formatação de Números e Valores**, **Manter Larguras das Colunas de Origem** e **Comentários e Notas**. São as mesmas opções de Colar Especial, com um clique cada.

A seta junto a **Copiar** tem **Copiar como Imagem** e **Copiar como Mapa de Bits**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada tal como as vê, com as linhas de grelha. Um mapa de bits tem o tamanho do ecrã. Uma imagem tem o dobro dos pontos, por isso mantém-se nítida quando é ampliada. **Copiar como Imagem Impressa** desenha as células como o papel as mostraria: as linhas de grelha só aparecem se a configuração da página as imprimir. Cole no JeffWrite ou no JeffDeck como imagem. Se as células não couberem numa só imagem, selecione menos.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por cada linha de células) e como tabela para páginas web, para que outros programas as possam usar. Quando cola algo que não veio do JeffCrunch, texto com tabulações ou uma tabela de uma página web ou de outra folha de cálculo, cada parte vai para a sua própria célula e é lida como se a tivesse escrito: números, datas e fórmulas que começam com um sinal de igual. Copiar de outro livro aberto no JeffCrunch traz valores, fórmulas e formatos de número, mas não a restante formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem entraria numa parte de uma **fórmula matricial**, sobre a linha com os nomes das colunas de uma **tabela**, em células bloqueadas de uma **folha protegida**, para além do fim da folha ou em mais de 100 000 células. Uma **fórmula matricial** completa pode ser cortada e movida (selecione todas as suas células); uma parte dela, uma tabela de dados ou a linha de nomes de uma tabela não podem.
