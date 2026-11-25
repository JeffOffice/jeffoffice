# Cortar, copiar e colar
<!-- keywords: cortar, copiar, colar, área de transferência, mover, duplicar, especial, valores, fórmulas, formatos, transpor, ignorar em branco, somar, subtrair, multiplicar, dividir, operação, unidas, texto, tabulação, html, página web, outro programa, Esc, formigas em marcha, limite, larguras das colunas, notas, comentários, formatos de número, todos exceto limites, imagem, mapa de bits, copiar como imagem, imagem impressa, como impresso, colar ligação, hiperligação, importação de texto, texto delimitado, largar texto, vírgula, ponto e vírgula, csv, separador, definir colagem predefinida, colagem predefinida, largura fixa -->

**Copiar** (**{key:Copy}**) coloca uma cópia das células selecionadas na área de transferência, um local que o computador mantém para uma coisa de cada vez. **Colar** (**{key:Paste}**) coloca o conteúdo da área de transferência nas células, a partir da célula selecionada. **Cortar** (**{key:Cut}**) é como copiar, mas, ao colar, as células são movidas e o local original fica vazio.

Um contorno tracejado em movimento mostra as células que foram copiadas ou cortadas. Prima **Esc** para retirar o contorno; as células deixam então de estar prontas para colar. As células cortadas também são libertadas se fizer qualquer outra alteração antes de as colar (escrever, inserir, anular): corte-as novamente se ainda quiser movê-las.

## Colar dentro do JeffCrunch

Tudo é transferido: os valores, as fórmulas, os formatos e as células unidas. Uma fórmula continua a apontar para os mesmos locais *em relação à sua posição*. Exemplo: copie **=A1*2** de B1 e cole em B5, e passa a ser **=A5*2**. Uma parte com um sinal de dólar, como **$A$1**, mantém-se como está. Se selecionar um bloco que seja um número inteiro de vezes o tamanho do que copiou, a cópia é repetida para o preencher: copie uma célula, selecione vinte células, cole, e todas as vinte recebem o conteúdo.

Um número colado que seja demasiado largo para uma coluna que ainda tenha a sua largura inicial alarga a coluna.

## Cortar move células

Quando corta células e as cola, as células são de facto movidas. Cada fórmula que apontava para elas passa a apontar para a sua nova posição. Exemplo: B1 tem **=A1+A2**. Corte A1:A2 e cole em E4, e B1 passa a ser **=E4+E5**. O que estava nas células onde colou é eliminado (**Anular** repõe), e uma fórmula que lia uma dessas células mostra **#REF!**, porque a célula que lia já não existe. Se as células cortadas forem apenas parte de uma célula unida, ou se colar sobre apenas parte de uma, nada é movido: selecione a célula unida inteira. Quando corta células para outra folha, as fórmulas continuam a ler as células que liam antes (indicam a folha). A área de transferência fica vazia após um corte e colagem.

## Colar Especial

**Ctrl+Alt+V**, ou a seta por baixo de **Colar** no separador Base, permite escolher o que colar:

- **Tudo**, **Fórmulas**, **Valores** ou **Formatos**. *Valores* transforma uma fórmula no número ou texto que apresenta.
- **Tudo Exceto Limites**: tudo, mas as células mantêm os limites que tinham.
- **Fórmulas e Formatos de Número**, ou **Valores e Formatos de Número**: as fórmulas (ou os valores) vêm com a forma como os números são apresentados, como percentagem ou data, mas não as cores, os tipos de letra ou os limites. As células mantêm o aspeto que tinham.
- **Larguras das Colunas**: as colunas onde colar ficam com as larguras das colunas copiadas.
- **Comentários e Notas**: as notas das células copiadas são colocadas nas células onde colar. Nenhum valor de célula muda.
- Uma **operação**: **Somar**, **Subtrair**, **Multiplicar** ou **Dividir**. Os números copiados são combinados com os números que já estão nas células. Exemplo: as células têm 10 e 20, copiou 5 e escolheu *Multiplicar*: passam a ser 50 e 100.
- **Ignorar células em branco**: as células vazias da cópia deixam as células de destino como estavam.
- **Transpor**: as linhas passam a colunas e as colunas a linhas.

## O botão Opções de Colagem

Logo após colar células copiadas no JeffCrunch, surge um pequeno botão **Opções de Colagem** junto à última célula colada. Clique nele para escolher outra forma de colocar as mesmas células: **Manter Formatação de Origem** (tudo), **Valores**, **Fórmulas**, **Formatação**, **Sem Limites**, **Fórmulas e Formatação de Números**, **Valores e Formatação de Números**, **Manter Larguras das Colunas de Origem**, ou **Transpor**. O JeffCrunch desfaz a colagem e refaz a operação dessa forma. A alteração conta como um só passo: um único **Anular** desfaz a colagem inteira, por mais vezes que tenha mudado de ideias. Exemplo: copie A1:B2, em que B1 é **=A1*2**, e cole em F5. Escolha **Valores** e G5 passa a conter o número 2 em vez de **=F5*2**; escolha **Manter Formatação de Origem** e a fórmula volta.

No teclado, prima a tecla **Ctrl** (a tecla **Control** num Mac) depois de colar para abrir o menu do botão e use as setas e **Enter** para escolher. O botão desaparece quando altera uma célula ou outra coisa no livro, copia outra coisa, prime **Esc**, ou quando a última célula colada sai do ecrã por deslocamento (volta quando voltar a deslocar). Não é oferecido após um corte e colagem, nem para texto de outro programa.

## Colar Ligação, Colar como Hiperligação e Colar Importação de Texto

Há mais três opções na seta por baixo de **Colar**. Funcionam com células copiadas no mesmo livro, exceto a última.

- **Colar Ligação** coloca em cada célula uma fórmula que aponta para a célula copiada. Exemplo: copie A1:B2, selecione D4 e use **Colar Ligação**: D4 passa a ser **=Folha1!$A$1**. Quando A1 mudar, D4 muda com ela. Uma célula copiada que esteja vazia mostra 0.
- **Colar como Hiperligação** coloca em cada célula as mesmas palavras que a célula copiada mostra, e transforma cada uma numa ligação para a célula copiada. Mantenha **Ctrl** premido e clique numa delas para ir para essa célula. Funciona com até 2 000 células de cada vez.
- **Colar Importação de Texto** serve para texto vindo de outro programa. Abre a mesma janela que abrir um ficheiro de texto: indica o que separa os elementos (uma vírgula, uma tabulação, um espaço, um ponto e vírgula, uma barra vertical ou outro carácter), o que delimita um elemento com aspas, a primeira linha a usar e a forma de ler cada coluna (tal como escrita, como texto, como data, ou omitida). Alguns textos alinham os elementos em colunas de largura igual, com espaços entre elas. Marque **As colunas têm uma largura fixa** e indique onde começa cada coluna: cada linha é cortada nesses pontos e os espaços à volta de cada elemento são retirados. Escolha *texto* para uma coluna de códigos como 007, para que os zeros se mantenham. Os elementos são colocados nas células, a partir da célula selecionada, como um único passo que pode anular.

## Texto com vírgulas, ponto e vírgula ou colunas

Quando o texto que cola de outro programa é claramente uma tabela, o JeffCrunch abre a mesma janela que ao abrir um ficheiro de texto, para que possa verificar como o texto está separado antes de qualquer coisa ser colocada nas células. Isto acontece apenas com um **Colar** normal (**{key:Paste}**), nunca com **Colar Especial** ou uma das suas opções. Acontece quando o texto tem pelo menos duas linhas e nenhuma tabulação, e quando, ou todas as linhas têm o mesmo número de elementos, pelo menos três, separados por vírgulas, pontos e vírgulas ou barras verticais, com algo em cada coluna (um elemento entre aspas pode conter uma vírgula); ou os elementos alinham-se em colunas de largura igual, com os mesmos intervalos de espaços em todas as linhas (pelo menos três linhas e pelo menos dois intervalos). Números com separadores de milhares, como 1,234, e linhas de código ou listas que terminam numa vírgula ou num ponto e vírgula não são considerados tabelas. A janela abre com a forma de separação que foi detetada. Se os números tiverem vírgulas e os elementos estiverem separados por pontos e vírgulas, a vírgula é o separador decimal. Clique em **OK** para colocar os elementos, ou em **Cancelar** para não colocar nada.

Exemplo: copie estas linhas de um correio eletrónico.

    nome,qtd,preço
    Maçã,12,1.50
    Pera,3,0.25

Se as colar com uma terceira coluna, como um preço, a janela mostra três colunas, com **Vírgula** marcada. Uma frase numa só linha, linhas com números diferentes de vírgulas, ou apenas dois elementos por linha são coladas como palavras, sem janela. O texto com tabulações entra diretamente nas células, como sempre aconteceu. Para abrir a janela para qualquer texto, escolha **Colar Importação de Texto** (acima).

O mesmo acontece quando arrasta texto de outro programa e o larga na grelha: entra na célula selecionada. Uma largada em qualquer outro ponto da janela não faz nada.

## Definir Colagem Predefinida

**Definir Colagem Predefinida** é a última opção na seta por baixo de **Colar**. Decide o que o botão **Colar** e **{key:Paste}** colocam quando copiou células: tudo (inicialmente), ou apenas os valores, apenas as fórmulas, apenas a formatação, e assim por diante. Exemplo: escolha *Valores*, e uma fórmula copiada entra como o número que mostra. A escolha fica guardada para a próxima vez. Não altera **Colar Especial** nem as opções do menu, e o texto de outros programas é colado tal como está. Escolha *Tudo* para voltar ao normal.

A mesma escolha está na página **Edição** de **Opções**, em duas caixas: uma para as células copiadas neste livro, e outra para as células copiadas noutro livro. Até escolher uma opção para outro livro, esta segue a primeira.

## O menu Colar e as imagens

A seta por baixo de **Colar** apresenta as opções habituais, e também **Sem Limites**, **Fórmulas e Formatação de Números**, **Valores e Formatação de Números**, **Manter Larguras das Colunas de Origem** e **Comentários e Notas**. São as mesmas opções de Colar Especial, com um clique cada.

A seta ao lado de **Copiar** tem **Copiar como Imagem** e **Copiar como Mapa de Bits**. Cada uma coloca na área de transferência uma imagem das células selecionadas, desenhada tal como as vê, com as linhas de grelha. Um mapa de bits tem o tamanho do ecrã. Uma imagem tem o dobro dos pontos, por isso fica nítida quando ampliada. **Copiar como Imagem Impressa** desenha as células tal como o papel as mostraria: as linhas de grelha só aparecem se a configuração da página as imprimir. Cole-a no JeffWrite ou no JeffDeck como imagem. Se as células não couberem numa só imagem, selecione menos.

## Entre programas

Ao copiar, o JeffCrunch também coloca as células na área de transferência como texto (células separadas por tabulações, uma linha por cada linha de células) e como tabela para páginas web, para que outros programas as possam usar. Quando cola algo que não veio do JeffCrunch, como texto com tabulações ou uma tabela de uma página web ou de outra folha de cálculo, cada elemento vai para a sua própria célula e é interpretado como se o tivesse escrito: números, datas e fórmulas que começam com um sinal de igual. Copiar de outro livro aberto no JeffCrunch traz valores, fórmulas e formatos de número, mas não a restante formatação. Cole-os no JeffDeck ou no JeffPub, e ficam como uma tabela que mantém os tipos de letra, os sombreados, os limites e os números tal como são apresentados (veja **Colar células como tabela** na Ajuda desses programas).

## O que não pode ser colado

O JeffCrunch avisa e não altera nada quando a colagem atingiria parte de uma **fórmula matricial**, a linha com os nomes das colunas de uma **tabela**, células bloqueadas de uma **folha protegida**, para além do fim da folha, ou mais de 100 000 células. Uma **fórmula matricial** completa pode ser cortada e movida (selecione todas as suas células); uma parte dela, uma tabela de dados ou a linha de nomes de uma tabela não podem.

## O painel Área de Transferência

O painel Área de Transferência guarda as últimas 24 coisas copiadas ou cortadas neste programa, para que possa colar uma mais antiga sem a copiar novamente. Clique na pequena seta no canto do grupo **Área de transferência** no separador **Base** (ou prima **Alt**, depois **H**, depois **FO**) para o abrir. Aparece um painel ao lado da página.

Cada item mostra uma pequena pré-visualização e o tipo de conteúdo: texto, texto formatado, células, uma imagem, formas ou diapositivos. O mais recente está no topo. Clique num item para o colar onde estiver o cursor ou a seleção. Cola como qualquer outra colagem: um passo de **Anular** desfaz-a, e o botão Opções de Colagem aparece como habitualmente.

- **Colar Tudo** cola todos os itens, do mais antigo para o mais recente, para que fiquem na ordem em que os copiou.
- **Eliminar** remove o item escolhido. **Limpar Tudo** remove todos.
- O painel só recolhe itens enquanto estiver aberto. Para continuar a recolher depois de o fechar, marque **Continuar a recolher quando este painel for fechado**. A escolha fica guardada, e a recolha começa quando o programa arranca, antes de abrir o painel.
- Para recolher também o que copia de outros programas, marque **Recolher também o que é copiado noutros programas**. Só texto simples e imagens são guardados, e esta opção volta a ficar desativada sempre que abre o programa. Com ela ativa, uma cópia feita noutro programa é lida por completo antes de este programa decidir se a guarda, por isso uma cópia muito grande pode fazer este programa parar por um momento e usar tanta memória quanto o tamanho da cópia. Só é lida uma forma: o seu texto, quando tem texto, ou, caso contrário, uma imagem. Com esta opção desativada, nada copiado noutros programas é lido.

Os itens são guardados apenas na memória do programa. Nada é gravado num ficheiro, e a lista fica vazia quando fecha a última janela do programa. Um item permanece na lista depois de o documento de origem ser fechado. Um item com mais de 8 MB não é guardado, e o painel avisa quando isso acontece; uma imagem conta 4 bytes por cada ponto (píxel), por isso uma imagem com mais de cerca de 2 milhões de píxeis é demasiado grande. Todos os itens juntos ficam limitados a 64 MB, e os mais antigos são removidos primeiro. Qualquer conteúdo que um gestor de palavras-passe marque como secreto nunca é guardado.
