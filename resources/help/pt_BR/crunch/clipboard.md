# Recortar, copiar e colar

<!-- keywords: recortar, copiar, colar, área de transferência, mover, duplicar, colar especial, valores, fórmulas, formatos, transpor, ignorar células vazias, somar, subtrair, multiplicar, dividir, operação, mesclada, texto, tabulação, html, página da web, outro programa, Esc, linha tracejada, borda, larguras de coluna, anotações, comentários, formatos de número, tudo exceto bordas, imagem, bitmap, copiar como imagem, imagem impressa, como impresso, colar vínculo, hiperlink, importar texto, texto delimitado, soltar texto, vírgula, ponto e vírgula, csv, separador, definir colagem padrão, colagem padrão, largura fixa -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um espaço que o computador guarda para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a partir da célula selecionada. **Recortar** (**{key:Cut}**) funciona como copiar, mas, ao colar, as células são movidas e o lugar antigo fica vazio.

Uma borda tracejada em movimento mostra as células que foram copiadas ou recortadas. Pressione **Esc** para retirar a borda; depois disso, as células não estão mais prontas para colar. Células recortadas também são liberadas quando você faz qualquer outra alteração antes de colá-las (digitar, inserir, desfazer): recorte-as de novo se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo é levado junto: os valores, as fórmulas, os formatos e as células mescladas. Uma fórmula continua apontando para os mesmos lugares *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e ela se torna **=A5*2**. Uma parte com cifrão, como **$A$1**, permanece como está. Se você selecionar um bloco que seja um número inteiro de vezes o tamanho do que foi copiado, a cópia se repete para preenchê-lo: copie uma célula, selecione vinte células, cole, e todas as vinte recebem o conteúdo.

Um número colado que seja largo demais para uma coluna que ainda tenha a largura inicial amplia a coluna.

## Recortar move células

Quando você recorta células e as cola, elas realmente se movem. Toda fórmula que apontava para elas passa a apontar para o novo lugar. Exemplo: B1 contém **=A1+A2**. Recorte A1:A2 e cole em E4, e B1 se torna **=E4+E5**. O que estava nas células sobre as quais você colou é perdido (**Desfazer** o traz de volta), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que ela lia não existe mais. Se as células recortadas forem apenas parte de uma célula mesclada, ou se você colar sobre apenas parte de uma delas, nada é movido: selecione a célula mesclada inteira. Ao recortar células para outra planilha, as fórmulas continuam lendo as células que liam antes (a planilha é indicada). A área de transferência é esvaziada após um recorte e uma colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta abaixo de **Colar** na guia Página Inicial, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que ela mostra.
- **Tudo Exceto Bordas**: tudo, mas as células mantêm as bordas que tinham.
- **Fórmulas e Formatos de Número**, ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são exibidos, como porcentagem ou data, mas não as cores, fontes ou bordas. As células mantêm a aparência que tinham.
- **Larguras de Coluna**: as colunas em que você cola recebem as larguras das colunas copiadas.
- **Comentários e Anotações**: as anotações das células copiadas são colocadas nas células em que você colou. Nenhum valor de célula muda.
- Uma **operação**: **Adicionar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números já existentes nas células. Exemplo: as células contêm 10 e 20, você copiou 5 e escolheu *Multiplicar*: elas passam a ter 50 e 100.
- **Ignorar células vazias**: células vazias na cópia deixam as células de destino como estavam.
- **Transpor**: linhas se tornam colunas e colunas se tornam linhas.

## O botão Opções de Colagem

Logo após colar células copiadas no JeffCrunch, um pequeno botão **Opções de Colagem** aparece junto à última célula colada. Clique nele para escolher outra forma de inserir as mesmas células: **Manter Formatação da Origem** (tudo), **Valores**, **Fórmulas**, **Formatação**, **Sem Bordas**, **Fórmulas e Formatação de Número**, **Valores e Formatação de Número**, **Manter Larguras de Coluna de Origem**, ou **Transpor**. O JeffCrunch desfaz a colagem e a refaz do jeito escolhido. A alteração conta como uma única etapa: um único **Desfazer** desfaz toda a colagem, por mais vezes que você tenha mudado de ideia. Exemplo: copie A1:B2, em que B1 é **=A1*2**, e cole em F5. Escolha **Valores** e G5 contém o número 2 em vez de **=F5*2**; escolha **Manter Formatação da Origem** e a fórmula volta.

Pelo teclado, toque na tecla **Ctrl** (a tecla **Control** em um Mac) após uma colagem para abrir o menu do botão e use as setas e **Enter** para escolher. O botão desaparece quando você altera uma célula ou qualquer outra coisa na pasta de trabalho, copia outra coisa, pressiona **Esc**, ou rola a última célula colada para fora da tela (ele volta ao rolar de volta). Ele não é oferecido após recortar e colar, nem para texto vindo de outro programa.

## Colar Vínculo, Colar como Hiperlink e Importar Texto ao Colar

Três outras opções estão na seta abaixo de **Colar**. Elas funcionam com células copiadas na mesma pasta de trabalho, exceto a última.

- **Colar Vínculo** coloca uma fórmula em cada célula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar Vínculo**: D4 se torna **=Planilha1!$A$1**. Quando A1 muda, D4 muda junto. Uma célula copiada que está vazia mostra 0.
- **Colar como Hiperlink** coloca nas células as mesmas palavras que a célula copiada mostra, e transforma cada uma em um link para a célula copiada. Mantenha **Ctrl** pressionada e clique em uma delas para ir até essa célula. Funciona com até 2.000 células por vez.
- **Importar Texto ao Colar** serve para texto vindo de outro programa. Abre a mesma janela de abrir um arquivo de texto: você informa o que separa as partes (vírgula, tabulação, espaço, ponto e vírgula, barra vertical ou outro caractere), o que delimita uma parte com aspas, a primeira linha a usar e como ler cada coluna (como digitado, como texto, como data ou ignorada). Alguns textos alinham suas partes em colunas de mesma largura, com espaços entre elas. Marque **As colunas têm largura fixa** e informe onde começa cada coluna: cada linha é cortada nesses pontos, e os espaços ao redor de cada parte são retirados. Escolha *texto* para uma coluna de códigos, como 007, para que os zeros sejam mantidos. As partes são inseridas nas células, a partir da célula selecionada, como uma única etapa que você pode desfazer.

## Texto com vírgulas, ponto e vírgula ou colunas

Quando o texto que você cola de outro programa é claramente uma tabela, o JeffCrunch abre a mesma janela de abrir um arquivo de texto, para que você possa conferir como ele foi separado antes de qualquer coisa entrar nas células. Isso acontece somente com **Colar** comum (**{key:Paste}**), nunca com **Colar Especial** ou uma de suas opções. Acontece quando o texto tem pelo menos duas linhas e nenhuma tabulação, e ocorre uma destas situações: cada linha tem o mesmo número de partes, pelo menos três, separadas por vírgulas, ponto e vírgula ou barras, com algum conteúdo em cada coluna (uma parte entre aspas pode conter vírgula); ou as partes se alinham em colunas de mesma largura, com os mesmos intervalos de espaços em todas as linhas (pelo menos três linhas e pelo menos dois intervalos). Números com separador de milhar, como 1.234, e linhas de código ou listas que terminam em vírgula ou ponto e vírgula não são considerados tabelas. A janela começa com a forma de separação identificada no texto. Se os números tiverem vírgulas e as partes forem separadas por ponto e vírgula, a vírgula é o separador decimal. Clique em **OK** para inserir as partes, ou em **Cancelar** para não inserir nada.

Exemplo: copie estas linhas de um e-mail.

    nome,qtd,preço
    Maçã,12,1.50
    Pera,3,0.25

Ao colá-las com uma terceira coluna, como um preço, a janela mostra três colunas, com **Vírgula** marcada. Uma frase em uma linha só, linhas com números diferentes de vírgulas, ou linhas com apenas duas partes são coladas como palavras, sem janela. Texto com tabulações vai direto para as células, como sempre foi. Para abrir a janela com qualquer texto, escolha **Importar Texto ao Colar** (acima).

O mesmo acontece quando você arrasta texto de outro programa e o solta na grade: ele é inserido a partir da célula selecionada. Soltar em qualquer outro ponto da janela não faz nada.

## Definir Colagem Padrão

**Definir Colagem Padrão** é a última opção na seta abaixo de **Colar**. Ela decide o que o botão **Colar** e **{key:Paste}** inserem quando você copiou células: tudo (no início), ou somente os valores, somente as fórmulas, somente a formatação, e assim por diante. Exemplo: escolha *Valores*, e uma fórmula copiada entra como o número que ela mostra. A escolha é mantida para a próxima vez. Ela não altera **Colar Especial** nem as opções do menu, e o texto de outros programas é colado como está. Escolha *Tudo* para voltar ao padrão.

A mesma escolha está na página **Edição** de **Opções**, em duas caixas: uma para células copiadas nesta pasta de trabalho, e outra para células copiadas em outra pasta de trabalho. Até que você escolha uma opção para outra pasta de trabalho, ela segue a primeira.

## O menu Colar e as imagens

A seta abaixo de **Colar** lista as opções habituais e também **Sem Bordas**, **Fórmulas e Formatação de Número**, **Valores e Formatação de Número**, **Manter Larguras de Coluna de Origem** e **Comentários e Anotações**. São as mesmas opções de Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como Imagem** e **Copiar como Bitmap**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada como você as vê, com as linhas de grade. Um bitmap tem o tamanho da tela. Uma imagem tem o dobro de pontos, por isso continua nítida quando ampliada. **Copiar como Imagem Impressa** desenha as células como o papel as mostraria: as linhas de grade aparecem somente se a configuração de página as imprimir. Cole na JeffWrite ou na JeffDeck como imagem. Se as células não couberem em uma única imagem, selecione menos células.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por linha) e como tabela para páginas da web, para que outros programas possam usá-las. Ao colar algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página da web ou de outra planilha, cada parte vai para a sua própria célula e é lida como se você a tivesse digitado: números, datas e fórmulas que começam com sinal de igual. Copiar de outra pasta de trabalho aberta no JeffCrunch traz valores, fórmulas e formatos de número, mas não as demais formatações. Ao colá-las na JeffDeck ou na JeffPub, elas entram como uma tabela que mantém as fontes, os sombreamentos, as bordas e os números como aparecem (veja **Colar células como tabela** na Ajuda desses programas).

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula de matriz**, a linha com os nomes de colunas de uma **tabela**, células bloqueadas de uma **planilha protegida**, além do fim da planilha, ou mais de 100.000 células. Uma **fórmula de matriz** inteira pode ser recortada e movida (selecione todas as suas células); parte dela, uma tabela de dados ou a linha de nomes de uma tabela não podem.

## O painel Área de Transferência

O painel Área de Transferência guarda os últimos 24 itens copiados ou recortados neste programa, para que você possa colar um item mais antigo sem copiá-lo de novo. Clique na pequena seta no canto do grupo **Área de Transferência** da guia Página Inicial (ou pressione **Alt**, depois **P**, depois **FO**) para abri-lo. Um painel aparece ao lado da página.

Cada item mostra uma pequena prévia e o tipo de conteúdo: texto, texto formatado, células, uma imagem, formas ou slides. O mais recente fica no topo. Clique em um item para colá-lo onde estão o cursor ou a seleção. Ele é colado como qualquer outra colagem: uma etapa de **Desfazer** o desfaz, e o botão Opções de Colagem aparece normalmente.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que fiquem na ordem em que foram copiados.
- **Excluir** remove o item selecionado. **Limpar Tudo** remove todos.
- O painel coleta itens somente enquanto estiver aberto. Para continuar coletando depois de fechá-lo, marque **Continuar coletando quando este painel for fechado**. A escolha fica salva, e a coleta começa quando o programa é iniciado, antes de você abrir o painel.
- Para coletar também o que você copia em outros programas, marque **Coletar também o que for copiado em outros programas**. Somente texto simples e imagens são guardados, e a opção é desligada de novo a cada vez que você abre o programa. Com ela ativada, uma cópia feita em outro programa é lida por inteiro antes que este programa decida se a guarda; por isso, uma cópia muito grande pode fazer este programa pausar por um momento e usar tanta memória quanto o tamanho da cópia. Apenas uma forma dela é lida: seu texto, quando há texto, ou então uma imagem. Com a opção desligada, nada copiado em outros programas é lido.

Os itens são guardados somente na memória do programa. Nada é salvo em arquivo, e a lista é esvaziada quando você fecha a última janela do programa. Um item permanece na lista mesmo depois de o documento de origem ser fechado. Um item maior que 8 MB não é guardado, e o painel avisa quando isso acontece; uma imagem conta 4 bytes por ponto (pixel), de modo que uma imagem com mais de cerca de 2 milhões de pixels é grande demais. Todos os itens juntos são limitados a 64 MB, e os mais antigos saem primeiro. Qualquer conteúdo que um gerenciador de senhas marque como secreto nunca é guardado.
