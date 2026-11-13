# Impressão em série, envelopes e etiquetas
<!-- keywords: impressão em série, unir, correspondência, destinatários, lista de endereços, carta modelo, etiquetas, envelopes, campo de impressão em série, bloco de endereço, linha de saudação, regras, se então senão, ignorar registo, registo seguinte, pré-visualizar resultados, concluir e unir, diretório, mensagens de correio eletrónico, csv, xlsx, ods, vcard -->

Uma **impressão em série** faz muitas cópias de uma carta, cada uma com os dados de uma pessoa diferente. Escreve a carta uma única vez. Guarda os dados das pessoas numa **lista**. O JeffWrite põe o nome e a morada da primeira pessoa na primeira cópia, os da segunda pessoa na segunda cópia, e assim por diante.

As ferramentas estão no separador **Correspondência**. Para um percurso guiado, clique em **Correspondência > Iniciar Correspondência > Assistente de Impressão em Série Passo a Passo**.

## Escolher o que está a criar

Clique em **Correspondência > Iniciar Correspondência** e escolha uma opção:

- **Cartas**: uma cópia da página para cada pessoa. Cada cópia começa numa página nova.
- **Mensagens de Correio Eletrónico**: uma mensagem para cada pessoa. Veja "Enviar mensagens de correio eletrónico" mais abaixo.
- **Envelopes** e **Etiquetas**: uma página feita para eles. Veja as secções seguintes.
- **Diretório**: uma lista de todas as pessoas, sem quebra de página entre elas. Use-a para uma lista telefónica ou um catálogo.
- **Limpar Tipo de Impressão em Série** volta a tornar o documento num documento normal.

O JeffWrite guarda dentro do ficheiro, ao gravar, o tipo de impressão em série e o nome da lista. Quando abrir o ficheiro novamente, a impressão em série continua lá.

## Escolher os destinatários

Um **destinatário** é uma pessoa que recebe uma cópia. Clique em **Correspondência > Selecionar Destinatários**:

- **Escrever uma Nova Lista**: escreva as pessoas na janela que abrir. Clique em **Nova Entrada** para cada uma. Depois, o JeffWrite oferece-se para guardar a lista como um ficheiro .csv.
- **Utilizar uma Lista Existente**: escolha um ficheiro que já seja uma lista. Pode ser um ficheiro .csv ou um ficheiro de texto com tabulações (uma linha por pessoa, e a primeira linha contém os nomes das colunas), uma folha de cálculo (.xlsx, .xls ou .ods, do JeffCrunch ou de outro programa) ou um ficheiro de contactos (.vcf).
- **Selecionar a partir dos Contactos**: escolha um ficheiro de contactos (.vcf). A maioria das agendas e dos telemóveis consegue guardar os contactos num ficheiro desse tipo.

Depois, clique em **Correspondência > Editar Lista de Destinatários** para verificar a lista:

- Desmarque uma linha para deixar essa pessoa de fora.
- Clique num cabeçalho de coluna para ordenar.
- Escreva em **Filtrar destinatários** para mostrar apenas as linhas que contêm as suas palavras.
- **Localizar Duplicados** desmarca as repetições.
- **Validar Endereços** coloca a cor nas linhas que não têm morada ou que têm um código postal com ar errado.

## Pôr os dados na carta

Um **campo de impressão em série** é um marcador de posição. Mostra o nome de uma coluna entre sinais de aspas angulares, como «Primeiro Nome». Em cada cópia, é substituído pelo dado dessa pessoa.

Clique onde o dado deve ir e use **Correspondência > Escrever e Inserir Campos**:

- **Inserir Campo de Impressão em Série** lista as colunas da sua lista. Escolha uma.
  - **Código de Barras a partir do Campo**, na mesma lista, insere um código de barras cujo valor é uma coluna, como um número de artigo ou o ISBN de 13 dígitos de um livro. Escolha a coluna e, depois, o tipo, e veja a janela **Inserir Código de Barras** (o que escrever aí é apenas um exemplo). A janela oferece só o que um campo consegue transportar: o tipo, o tamanho, a altura das barras, os dígitos e o nível do QR. Deixa de fora o preço de um livro, um suplemento, um carácter de controlo e o fundo limpo, que um campo não consegue indicar; o ISBN de um livro é um EAN-13. No documento, o campo mostra o nome da coluna entre aspas angulares. Ao unir, a cópia de cada pessoa recebe um código de barras com o valor dessa pessoa, desenhado como imagem na linha, para que seja impresso e guardado em PDF como qualquer imagem. O valor é apenas dados: aspas ou uma barra invertida nele não alteram nada. Se um valor estiver vazio, não há código de barras. Se um código não conseguir conter o valor (uma letra num EAN-13, ou mais de 500 caracteres num Code 128), a cópia mostra uma breve mensagem entre parênteses retos no seu lugar, e as outras cópias não são afetadas. Uma impressão em série cria no máximo 2 000 códigos de barras, contando cada valor que não consiga ser representado; se indicar que há demasiados, una menos pessoas. Num `.docx` guardado, o campo é um campo DISPLAYBARCODE com a coluna lá dentro, por isso mantém-se ao gravar e voltar a abrir o ficheiro. Um campo com um valor escrito diretamente, ou de um tipo que o JeffWrite não desenha (UPC-E, por exemplo), fica como está. As impressões em série de mensagens de correio eletrónico põem o valor como texto.
- **Bloco de Endereço** insere uma morada completa: o nome, a empresa, a rua e a localidade. Escolha as linhas que pretende. Uma linha sem conteúdo é omitida. Também pode indicar um país a omitir, como o seu próprio. A **Pré-visualização** mostra a morada da primeira pessoa.
- **Linha de Saudação** insere uma frase como "Caro Dr. Fenwick,". Escolhe as palavras. A primeira opção, "Caro Dr. Fenwick,", usa o tratamento da pessoa. Se a pessoa não tiver tratamento, usa o nome próprio e o apelido, como em "Cara Nora Pell,". Se uma pessoa não tiver nome, a saudação diz outra coisa, como "Caro Senhor ou Senhora,", que pode alterar.
- **Corresponder Campos** indica ao bloco de endereço e à linha de saudação qual coluna contém cada parte, quando os nomes das suas colunas são pouco comuns.

## Regras

Uma **regra** torna uma cópia diferente para algumas pessoas. Clique em **Regras** e escolha uma:

- **Se…Então…Senão**: mostra um texto quando uma coluna tem um valor, e outro texto quando não tem. Por exemplo, se o Distrito for "Lisboa", escreva "Bem-vindo a casa".
- **Ignorar Registo Se**: deixa uma pessoa de fora quando uma coluna tem um valor, como quando o apelido está vazio.
- **Registo Seguinte** e **Registo Seguinte Se**: usam os dados da pessoa seguinte no mesmo lugar. As etiquetas usam isto para pôr uma pessoa diferente em cada etiqueta.
- **N.º de Registo da Impressão em Série** insere o número da linha da pessoa na lista. **N.º de Sequência de Impressão em Série** conta as cópias feitas até ao momento.
- **Perguntar** e **Campo a Preencher**: fazem-lhe uma pergunta quando termina a impressão em série. Perguntar guarda a resposta com um nome. Campo a Preencher coloca a resposta no texto.
- **Definir Marcador**: atribui um valor a um nome. Uma referência cruzada a esse nome mostra o valor.

## Pré-visualizar os resultados

Clique em **Correspondência > Pré-visualizar Resultados**. Abre-se um painel que mostra a página de uma pessoa. Use **Primeiro Registo**, **Registo Anterior**, **Registo Seguinte** e **Último Registo** para percorrer as pessoas, ou escreva um número. **Localizar Destinatário** salta para a pessoa seguinte que tenha as suas palavras num dado.

**Realçar Campos de Impressão em Série** sombreia a cinzento os campos de impressão em série na página, para ver onde estão. Os outros campos, como os números de página, não são sombreados. O sombreado aparece no ecrã e nunca é impresso.

O seu documento não é alterado. **Verificar Erros** avisa quando a lista não tem uma coluna para um campo, ou quando não está incluída ninguém.

## Concluir a impressão em série

Clique em **Correspondência > Concluir e Unir**:

- **Editar Documentos Individuais** cria um novo documento com a cópia de cada pessoa. Pode alterá-lo antes de imprimir.
- **Imprimir Documentos** cria esse documento e abre a página **Imprimir**.
- **Guardar como PDF** guarda todas as cópias num único ficheiro PDF.
- **Enviar Mensagens de Correio Eletrónico** cria um ficheiro de mensagem .eml para cada pessoa, numa pasta que escolher. Escolha a coluna que contém os endereços de correio eletrónico e escreva um assunto. O JeffWrite não as envia. Abra os ficheiros no seu programa de correio eletrónico.

As pessoas que desmarcou na lista ficam de fora.

Um ficheiro de mensagem só é criado para uma pessoa cujo endereço seja um único endereço de correio eletrónico. Se o endereço tiver dois (com uma vírgula ou um ponto e vírgula entre eles), tiver um nome associado, ou não tiver domínio, essa pessoa não recebe mensagem, e o JeffWrite indica quantas são. Um assunto longo é guardado em partes, que os programas de correio eletrónico voltam a juntar.

Uma impressão em série mostra uma janela de progresso, e **Cancelar** interrompe-a. Se uma regra fizer uma pergunta (Perguntar ou Campo a Preencher), é-lhe feita cada pergunta diferente uma vez, até 20. Depois disso, as restantes usam as respostas predefinidas. Se cancelar uma pergunta, a impressão em série inteira pára. A impressão em série para e indica-o se for criar mais de 10 000 páginas, para que um documento muito longo com uma lista muito grande não demore horas. Una uma parte da lista de cada vez.

## Envelopes

Clique em **Correspondência > Envelopes**. Escreva a morada de entrega e a sua morada de remetente, e escolha o tamanho do envelope na lista. Os tamanhos são indicados pela largura e pela altura. Clique em **Adicionar ao Documento** para colocar o envelope na sua própria página, no início do documento, ou em **Imprimir** para o imprimir. A página tem a largura do lado maior do envelope, que é a forma como a maioria das impressoras recebe um envelope.

Para criar envelopes para uma impressão em série, clique em **Iniciar Correspondência > Envelopes**. Cada envelope tem um bloco de endereço.

## Etiquetas

Clique em **Correspondência > Etiquetas**. Escreva a morada e escolha a folha na lista. Uma folha é indicada pelo número de etiquetas que tem e pelo tamanho de cada etiqueta. Se a sua folha não estiver na lista, escolha **Folha personalizada** e escreva as medidas: quantas etiquetas cabem na horizontal e na vertical, o tamanho de uma etiqueta, o espaço acima e ao lado da primeira etiqueta, e a distância entre as etiquetas.

- **Página completa da mesma etiqueta** coloca a mesma morada em todas as etiquetas da folha.
- **Etiqueta única** coloca a morada numa só etiqueta, para aproveitar uma folha já parcialmente usada. Indique a linha e a coluna.

Clique em **Novo Documento** para criar um documento que possa ver e alterar, ou em **Imprimir** para o imprimir.

Para criar etiquetas para uma impressão em série, clique em **Iniciar Correspondência > Etiquetas**. A primeira etiqueta tem um bloco de endereço, e cada uma das outras começa com uma regra **Registo Seguinte**. Se alterar a primeira etiqueta, clique em **Atualizar Etiquetas** para copiar a alteração para todas as outras.

## O que o ficheiro guarda

O tipo de impressão em série, o nome e o local da lista, o assunto e a coluna de correio eletrónico ficam guardados no seu ficheiro .docx. As pessoas que desmarcou e as suas escolhas em Corresponder Campos não são guardadas. São pedidas novamente depois de abrir o ficheiro. Uma lista é lida de novo a partir do seu ficheiro sempre que é usada. Se o ficheiro não estiver onde estava, o JeffWrite procura ao lado do seu documento e, depois, pede-lhe que o escolha.

Uma lista pode ter até 1 000 colunas e 4 000 000 de células no total, e uma impressão em série usa as primeiras 100 000 pessoas. Se um ficheiro tiver mais, o JeffWrite usa o que cabe e indica-o. As linhas que não têm nada são omitidas da lista.

## Consulte também

- [Utilizar campos](fields)
- [Imprimir](print)
- [Criar e alterar tabelas](tables)
