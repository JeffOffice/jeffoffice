# Alterar maiúsculas, ordenar e colar de outras formas

<!-- keywords: botão colar, recortar e colar inteligente, espaços ao colar, linha horizontal, régua, mostrar formatação, inspetor de estilos, shift+f1, tabulações, posições de tabulação, carácter de preenchimento, manter com o seguinte, manter linhas juntas, quebra de página antes, linha isolada, linha órfã, nível de contorno, quebras de linha e de página, maiúsculas e minúsculas, maiúsculas, minúsculas, maiúscula inicial da frase, maiúscula inicial, alternar maiúsculas, shift+f3, ordenar, ordenar parágrafos, alfabética, ordem, colar, colar especial, continuar lista, nova lista, unir lista, aninhar tabela, células de folha de cálculo, manter apenas texto, unir formatação, rtf, texto formatado, arrastar texto, texto sem formatação, imagem, área de transferência -->

Estas ferramentas estão no separador **Base**. Funcionam sobre o texto que seleciona.

## Alterar as maiúsculas e minúsculas das letras

A **capitalização** de uma letra indica se é maiúscula (A) ou minúscula (a).

1. Selecione o texto. Sem nada selecionado, o JeffWrite altera a palavra onde está o cursor.
2. No grupo **Tipo de Letra**, clique em **Maiúsculas/Minúsculas**.
3. Escolha uma opção:
   - **Maiúscula inicial da frase** põe em maiúscula a primeira letra de cada frase. As restantes ficam em minúsculas.
   - **minúsculas** põe todas as letras em minúsculas.
   - **MAIÚSCULAS** põe todas as letras em maiúsculas.
   - **Maiúscula Inicial das Palavras** põe em maiúscula a primeira letra de cada palavra.
   - **aLTERNAR mAIÚSCULAS** transforma cada maiúscula em minúscula, e cada minúscula em maiúscula.

Prima **Shift+F3** para percorrer três destas opções por ordem: **Maiúscula Inicial das Palavras**, depois **MAIÚSCULAS**, depois **minúsculas**.

O aspeto das letras mantém-se. O negrito continua negrito. **Ctrl+Z** anula a alteração num só passo. Se o Controlo de Alterações estiver ativado, a alteração aparece como uma eliminação e uma inserção.

## Ordenar parágrafos

Ordenar coloca os parágrafos por ordem, como de A a Z. Use para uma lista de nomes ou para linhas de uma tabela feita com tabulações ou vírgulas.

1. Selecione dois ou mais parágrafos.
2. No grupo **Parágrafo**, clique em **Ordenar**.
3. Escolha o critério de ordenação. Cada escolha tem um **campo**, um **tipo** e uma **ordem**.
   - O **campo** é uma parte do parágrafo. O campo 1 é a primeira parte. Imagine que as partes estão separadas por vírgulas ou tabulações, na caixa abaixo das chaves. Se deixar em **Parágrafos**, o parágrafo inteiro é o único campo.
   - O **tipo** é **Texto**, **Número** ou **Data**. Um número ordena-se pelo seu valor, por isso 9 vem antes de 10.
   - A **ordem** é **Crescente** (de A a Z, do menor para o maior, do mais antigo para o mais recente) ou **Decrescente**.
4. Use **Depois por** para desempatar.
5. Marque **O primeiro parágrafo é um cabeçalho** para o deixar no lugar.
6. Clique em **OK**.

Cada parágrafo mantém o seu próprio aspeto. Não é possível ordenar com o Controlo de Alterações ativado, porque uma ordenação não é uma alteração que possa ser mostrada. Desative o Controlo de Alterações, ordene e volte a ativá-lo.

## Colar de outras formas

Clique na pequena seta abaixo de **Colar** para mais opções:

- Quando o texto vem de outro programa que coloca texto formatado (RTF) na área de transferência, **Ctrl+V** mantém o seu aspeto: tipos de letra, tamanhos, cores, negrito, listas, tabelas, ligações e imagens. Se o programa não deu texto formatado, o JeffWrite usa a forma de página Web (HTML) e, por fim, apenas as palavras. Texto formatado com mais de 16 MB não é lido, e a colagem usa a forma seguinte. Arrastar texto formatado de outro programa para a página faz o mesmo.
- **Unir Formatação** cola o texto com o aspeto do local para onde vai. Apenas o negrito, o itálico, o sublinhado e o riscado são mantidos.
- **Manter Apenas Texto** cola as palavras sem qualquer aspeto.
- **Imagem** cola uma imagem da área de transferência. Fica esbatida quando a área de transferência não tem nenhuma imagem.
- **Colar Especial** (**Ctrl+Alt+V**) lista todas as formas como a área de transferência pode ser colada neste momento, e deixa escolher uma, como **Texto formatado (RTF)**, **Texto sem formatação** ou **HTML**.

Uma opção fica esbatida quando a área de transferência não tem nada que possa usar.

Logo após colar, aparece junto ao cursor um pequeno botão **Opções de Colagem**. Clique nele para escolher **Manter Formatação de Origem**, **Unir Formatação** ou **Manter Apenas Texto**. O JeffWrite desfaz a colagem e refaz-na dessa forma. O botão desaparece assim que escrever ou mover o cursor.

Quando a colagem trouxe parágrafos de lista junto de uma lista, o botão oferece também **Continuar Lista** (a numeração continua a partir da lista ao lado), **Nova Lista** (a numeração recomeça em 1) e **Unir Lista** (os itens juntam-se à lista ao lado e tomam o seu aspeto). **Ctrl+V** simples coloca os itens tal como vieram, e uma escolha altera isso. As mesmas opções são comandos no menu Colar do separador Base e em **Colar Especial**, por isso também podem ser usadas pelo teclado.

Quando a área de transferência tem células de folha de cálculo (copiadas do JeffCrunch ou de outro programa como tabela ou como linhas com tabulações entre as células), o **Ctrl+V** simples cola-as como as palavras que são. Para as colar como células, escolha **Tabela** no botão, no menu Colar ou em **Colar Especial**. Com o cursor dentro de uma tabela, o botão oferece **Aninhar Tabela** (uma tabela dentro da célula), **Unir Tabela** (as células juntam-se à tabela, que recebe as colunas de que precisa), **Inserir como Novas Linhas** e **Substituir Células** (as células substituem as da tabela, a partir da célula do cursor). Uma tabela colada tem no máximo 500 linhas e 63 colunas, e uma célula tem no máximo 4000 carateres; o que ultrapassar isso fica de fora, e uma mensagem avisa. Uma tabela aninhada dentro de outra pode ter, no máximo, 8 níveis de profundidade, e uma tabela aninhada cujas colunas ficassem demasiado estreitas para a célula é recusada. Cada colagem é um só passo: **Anular** desfaz tudo, e **Refazer** volta a colocá-lo.

**Recortar e colar inteligente** mantém os espaços corretos ao mover palavras. Se recortar palavras completas que estão entre dois espaços, o JeffWrite leva também um desses espaços, para não ficar com dois espaços seguidos. Se colar uma palavra encostada a outra, acrescenta um espaço entre elas. Não altera o meio de uma palavra. Para desativar esta opção, clique em **Ficheiro > Opções > Avançadas** e desmarque **Recortar e colar inteligente**.

## Quebras de linha e de página, e tabulações

Clique no pequeno botão no canto do grupo **Parágrafo** para abrir a caixa **Parágrafo**. Por baixo das opções de espaçamento estão estas:

- **Controlo de linhas isoladas e órfãs** evita que uma linha solta de um parágrafo fique sozinha no topo ou na base de uma página.
- **Manter com o seguinte** mantém o parágrafo na mesma página que o seguinte. Use-o para um título.
- **Manter linhas juntas** mantém todas as linhas do parágrafo numa só página.
- **Suprimir números de linha** deixa este parágrafo de fora quando as linhas são numeradas por página (veja Esquema > Números de Linha).
- **Quebra de página antes** começa o parágrafo numa nova página.
- **Não hifenizar** impede que as palavras do parágrafo sejam divididas com um hífen no fim de uma linha.
- **Não adicionar espaço entre parágrafos do mesmo estilo** remove o espaço entre dois parágrafos que têm o mesmo estilo.
- **Nível de contorno** indica onde está o parágrafo na estrutura do documento: **Corpo de texto**, ou um nível de 1 a 9.

Clique em **Tabulações** na caixa para definir posições de tabulação. Uma posição de tabulação é um lugar para onde o texto salta quando prime **Tab**. Escreva a sua posição, escolha a forma como o texto se alinha ali (à esquerda, ao centro, à direita ou decimal) e escolha um carácter de preenchimento. Um carácter de preenchimento é a linha de pontos, traços ou um traço contínuo que preenche o espaço antes da posição. Clique em **Definir** para a acrescentar. Para a remover, selecione-a na lista e clique em **Limpar**. **Limpar Tudo** remove todas.

## Desenhar uma linha horizontal

Clique em **Linha Horizontal** no grupo **Parágrafo**. O JeffWrite acrescenta um parágrafo próprio com uma linha fina por baixo, depois do parágrafo onde está o cursor. O cursor passa para um parágrafo simples abaixo da linha. Para retirar a linha, selecione o respetivo parágrafo e elimine-o.

## Ver o que está definido onde está o cursor

Prima **Shift+F1**, ou use **Mostrar Formatação**. Uma caixa indica os estilos, o tipo de letra, o tamanho, o aspeto das letras, a cor, e o alinhamento, os avanços e o espaçamento do parágrafo na posição do cursor. Apenas mostra estes valores. Para os alterar, use as caixas **Tipo de Letra** e **Parágrafo**.
