# Alterar maiúsculas e minúsculas, classificar e colar de outras formas

<!-- keywords: botão opções de colagem, recortar e colar inteligente, espaços ao colar, linha horizontal, régua, revelar formatação, inspetor de estilos, shift+f1, tabulações, paradas de tabulação, preenchimento, manter com o próximo, manter linhas juntas, quebra de página antes, viúva, órfã, nível de estrutura de tópicos, quebras de linha e de página, alterar maiúsculas e minúsculas, maiúsculas, minúsculas, maiúscula no início da frase, iniciais maiúsculas, alternar maiúsculas e minúsculas, shift+f3, classificar, classificar parágrafos, alfabética, ordem, colar, colar especial, continuar lista, nova lista, mesclar lista, aninhar tabela, células de planilha, manter somente texto, mesclar formatação, rtf, texto formatado, soltar, arrastar texto, texto não formatado, imagem, área de transferência -->

Estas ferramentas estão na guia **Página Inicial**. Elas funcionam sobre o texto que você seleciona.

## Alterar as maiúsculas e minúsculas das letras

A **caixa** de uma letra indica se ela é maiúscula (A) ou minúscula (a).

1. Selecione o texto. Sem nada selecionado, o JeffWrite altera a palavra onde está o cursor.
2. No grupo **Fonte**, clique em **Maiúsculas e Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula no início da frase** deixa a primeira letra de cada frase maiúscula. O restante fica minúsculo.
   - **minúsculas** deixa todas as letras minúsculas.
   - **MAIÚSCULAS** deixa todas as letras maiúsculas.
   - **Iniciais Maiúsculas** deixa a primeira letra de cada palavra maiúscula.
   - **aLTERNAR mAIÚSC./mINÚSC.** transforma cada maiúscula em minúscula, e cada minúscula em maiúscula.

Pressione **Shift+F3** para percorrer três delas em sequência: **Iniciais Maiúsculas**, depois **MAIÚSCULAS**, depois **minúsculas**.

O aspecto das letras não muda. O negrito continua negrito. **Ctrl+Z** desfaz a alteração em um único passo. Se o Controlar Alterações estiver ativado, a mudança aparece como uma exclusão e uma inserção.

## Classificar parágrafos

Classificar coloca os parágrafos em ordem, como de A a Z. Use para uma lista de nomes ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Classificar**.
3. Escolha o critério de classificação. Cada opção tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Considere que as partes são separadas por vírgulas ou tabulações, na caixa abaixo das chaves. Se você deixar como **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número é classificado pelo seu valor, então 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, do mais antigo para o mais recente) ou **Decrescente**.
4. Use **Em seguida por** para desempatar.
5. Marque **A primeira linha é um cabeçalho** para mantê-la onde está.
6. Clique em **OK**.

Cada posição mantém a sua própria aparência de parágrafo. Não é possível classificar com o Controlar Alterações ativado, pois uma classificação não é uma alteração a ser mostrada. Desative o Controlar Alterações, classifique e ative-o de novo.

## Colar de outras formas

Clique na pequena seta abaixo de **Colar** para ver mais opções:

- Quando o texto vem de outro programa que coloca texto formatado (RTF) na área de transferência, **Ctrl+V** mantém a aparência: fontes, tamanhos, cores, negrito, listas, tabelas, links e imagens. Se o programa não forneceu texto formatado, o JeffWrite usa a forma de página da web (HTML) e, depois, apenas as palavras. Texto formatado com mais de 16 MB não é lido, e a colagem usa a próxima forma. Soltar texto formatado de outro programa na página faz o mesmo.
- **Mesclar Formatação** cola o texto com a aparência do local para onde vai. Só o negrito, o itálico, o sublinhado e o tachado são mantidos.
- **Manter Somente Texto** cola as palavras sem nenhuma aparência.
- **Imagem** cola uma imagem da área de transferência. Ela fica esmaecida quando a área de transferência não tem imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada agora, e permite escolher uma, como **Texto formatado (RTF)**, **Texto não formatado** ou **HTML**.

Uma opção fica esmaecida quando a área de transferência não tem nada que ela possa usar.

Logo após colar, um pequeno botão **Opções de Colagem** aparece ao lado do cursor. Clique nele para escolher **Manter Formatação de Origem**, **Mesclar Formatação** ou **Manter Somente Texto**. O JeffWrite desfaz a colagem e a refaz desse jeito. O botão desaparece assim que você digitar ou mover o cursor.

Quando a colagem trouxe parágrafos de lista ao lado de uma lista, o botão também oferece **Continuar Lista** (a numeração continua a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Mesclar Lista** (os itens se juntam à lista ao lado e assumem a sua aparência). O **Ctrl+V** comum insere os itens como vieram, e uma escolha altera isso. As mesmas opções são comandos no menu Colar da guia Página Inicial e em **Colar Especial**, então o teclado também chega a elas.

Quando a área de transferência contém células de planilha (copiadas do JeffCrunch ou de outro programa como uma tabela ou como linhas com tabulações entre as células), o **Ctrl+V** comum as cola como as palavras que são. Para colá-las como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Mesclar Tabela** (as células se juntam à tabela, que ganha as colunas de que precisa), **Inserir como Novas Linhas** e **Substituir Células** (as células substituem as da própria tabela, a partir da célula do cursor). Uma tabela colada tem no máximo 500 linhas e 63 colunas, e uma célula contém no máximo 4.000 caracteres; o que passar disso é deixado de fora, e uma mensagem avisa. Uma tabela aninhada em outra pode ter no máximo 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficariam estreitas demais para a célula é recusada. Cada colagem é um único passo: Desfazer desfaz tudo, e Refazer o coloca de volta.

**Recortar e colar inteligente** mantém os espaços corretos ao mover palavras. Se você recortar palavras inteiras que estão entre dois espaços, o JeffWrite também remove um desses espaços, para que não sobre um espaço duplo. Se você colar uma palavra colada em outra palavra, ele acrescenta um espaço entre elas. Não altera o meio de uma palavra. Para desativar, clique em **Arquivo > Opções > Avançado** e desmarque **Recortar e colar inteligente**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Abaixo das opções de espaçamento estão estas:

- **Controle de viúvas e órfãs** impede que uma linha isolada de um parágrafo fique sozinha no início ou no fim de uma página.
- **Manter com o próximo** mantém o parágrafo na mesma página do seguinte. Use em títulos.
- **Manter linhas juntas** mantém todas as linhas do parágrafo em uma só página.
- **Suprimir números de linha** deixa este parágrafo de fora quando as linhas de página são numeradas (veja Layout > Números de Linha).
- **Quebra de página antes** começa o parágrafo em uma nova página.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de estrutura de tópicos** indica onde o parágrafo está na estrutura do documento: **Corpo de texto**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir paradas de tabulação. Uma parada de tabulação é um lugar para onde o texto pula quando você pressiona **Tab**. Digite a posição, escolha como o texto se alinha ali (à esquerda, centralizado, à direita ou decimal) e escolha um preenchimento. O preenchimento é a linha de pontos, traços ou um traço contínuo que preenche o espaço antes da parada. Clique em **Definir** para acrescentá-la. Para remover uma, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite acrescenta um parágrafo próprio com uma linha fina embaixo, depois do parágrafo onde está o cursor. O cursor passa para um parágrafo comum abaixo da linha. Para retirar a linha, selecione o parágrafo dela e exclua-o.

## Ver o que está definido onde está o cursor

Pressione **Shift+F1** ou use **Revelar Formatação**. Uma caixa lista os estilos, a fonte, o tamanho, a aparência das letras, a cor e o alinhamento, os recuos e o espaçamento do parágrafo no cursor. Ela apenas mostra essas informações. Para alterá-las, use as caixas **Fonte** e **Parágrafo**.
