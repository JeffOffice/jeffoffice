# Recortar, copiar e colar

<!-- keywords: recortar, copiar, colar, área de transferência, mover, duplicar, colar especial, valores, fórmulas, formatos, transpor, ignorar em branco, somar, subtrair, multiplicar, dividir, operação, mesclada, mesclado, texto, tabulação, html, página da web, outro programa, Esc, formigas marchantes, borda, larguras de coluna, anotações, comentários, formatos de número, tudo exceto bordas, imagem, bitmap, copiar como imagem -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, começando pela célula selecionada. **Recortar** (**{key:Cut}**) funciona como copiar, mas, ao colar, as células são movidas e o local original fica vazio.

Uma borda tracejada em movimento mostra as células copiadas ou recortadas. Pressione **Esc** para remover a borda; depois disso, as células não estão mais prontas para serem coladas. As células recortadas também são liberadas quando você faz outra alteração antes de colá-las (digitando, inserindo ou desfazendo): recorte-as novamente se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo é colado: os valores, as fórmulas, os formatos e as células mescladas. Uma fórmula continua apontando para os mesmos lugares *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e ela se torna **=A5*2**. Uma parte com cifrão, como **$A$1**, permanece como está. Se você selecionar um bloco que seja um número inteiro de vezes o tamanho do que foi copiado, a cópia se repete para preenchê-lo: copie uma célula, selecione vinte células, cole, e todas as vinte recebem o conteúdo.

Um número colado que seja largo demais para uma coluna que ainda tem a largura inicial alarga a coluna.

## Recortar move células

Quando você recorta células e as cola, elas são realmente movidas. Toda fórmula que apontava para elas passa a apontar para o novo lugar. Exemplo: B1 contém **=A1+A2**. Recorte A1:A2 e cole em E4, e B1 se torna **=E4+E5**. O que estava nas células sobre as quais você colou é perdido (**Desfazer** o traz de volta), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que ela lia não existe mais. Se as células recortadas forem apenas parte de uma célula mesclada, ou se você colar sobre apenas parte de uma delas, nada é movido: selecione a célula mesclada inteira. Quando você recorta células para outra planilha, as fórmulas continuam lendo as células que liam antes (a planilha é indicada). A área de transferência é esvaziada depois de um recorte e uma colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta abaixo de **Colar** na guia Página Inicial, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que ela exibe.
- **Tudo Exceto Bordas**: tudo, mas as células mantêm as bordas que tinham.
- **Fórmulas e Formatos de Número**, ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são exibidos, como porcentagem ou data, mas não as cores, fontes ou bordas. As células mantêm a aparência que tinham.
- **Larguras de Coluna**: as colunas em que você colar recebem as larguras das colunas copiadas.
- **Comentários e Anotações**: as anotações das células copiadas são colocadas nas células em que você colar. Nenhum valor de célula muda.
- Uma **operação**: **Somar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células contêm 10 e 20, você copiou 5, e escolheu *Multiplicar*: elas passam a ter 50 e 100.
- **Ignorar em branco**: células vazias na cópia deixam as células de destino como estavam.
- **Transpor**: linhas viram colunas e colunas viram linhas.

## O menu Colar e as imagens

A seta abaixo de **Colar** lista as opções habituais e também **Sem Bordas**, **Fórmulas e Formatação de Número**, **Valores e Formatação de Número**, **Manter Larguras de Coluna de Origem** e **Comentários e Anotações**. São as mesmas opções do Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como Imagem** e **Copiar como Bitmap**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada como você as vê, com as linhas de grade. Um bitmap tem o tamanho da tela. Uma imagem tem o dobro de pontos, por isso continua nítida quando é ampliada. Cole-a no JeffWrite ou no JeffDeck como imagem. Se as células não couberem em uma única imagem, selecione menos células.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por linha) e como tabela para páginas da web, para que outros programas possam usá-las. Quando você cola algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página da web ou de outra planilha, cada parte vai para a sua própria célula e é lida como se você a tivesse digitado: números, datas e fórmulas que começam com sinal de igual. Copiar de outra pasta de trabalho aberta no JeffCrunch traz valores, fórmulas e formatos de número, mas não a outra formatação.

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula de matriz**, a linha de nomes de colunas de uma **tabela**, células bloqueadas de uma **planilha protegida**, além do fim da planilha, ou mais de 100.000 células. Uma fórmula de matriz inteira pode ser recortada e movida (selecione todas as suas células); parte dela, uma tabela de dados ou a linha de nomes de uma tabela não podem.
