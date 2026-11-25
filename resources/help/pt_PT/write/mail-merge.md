# Mala direta, envelopes e etiquetas
<!-- keywords: impressão em série, mala direta, correspondência, destinatários, lista de endereços, carta modelo, etiquetas, envelopes, campo de impressão em série, bloco de endereço, linha de saudação, regras, se então senão, ignorar registo, registo seguinte, pré-visualizar resultados, concluir e unir, diretório, mensagens de correio eletrónico, csv, xlsx, ods, vcard -->

Uma **impressão em série** faz muitas cópias de uma só carta, cada uma com os dados de uma pessoa diferente. Escreve a carta uma vez. Guarda os dados das pessoas numa **lista**. O JeffWrite coloca o nome e a morada da primeira pessoa na primeira cópia, os da segunda pessoa na segunda cópia, e assim sucessivamente.

As ferramentas estão no separador **Correspondência**. Para um percurso guiado, clique em **Correspondência > Iniciar Correspondência > Assistente de Impressão em Série Passo a Passo**.

## Escolha o que vai criar

Clique em **Correspondência > Iniciar Correspondência** e escolha uma opção:

- **Cartas**: uma cópia da página para cada pessoa. Cada cópia começa numa página nova.
- **Mensagens de Correio Eletrónico**: uma mensagem para cada pessoa. Veja "Enviar mensagens de correio eletrónico" mais abaixo.
- **Envelopes** e **Etiquetas**: uma página preparada para estes. Veja as secções seguintes.
- **Diretório**: uma lista de todas as pessoas, sem quebra de página entre elas. Use-o para uma lista telefónica ou um catálogo.
- **Limpar Tipo de Impressão em Série** volta a tornar o documento um documento normal.

O JeffWrite guarda no ficheiro o tipo de impressão e o nome da lista quando guarda o documento. Quando abre o ficheiro novamente, a impressão em série continua lá.

## Escolha os destinatários

Um **destinatário** é uma pessoa que recebe uma cópia. Clique em **Correspondência > Selecionar Destinatários**:

- **Escrever uma Nova Lista**: escreva as pessoas na janela que se abre. Clique em **Nova Entrada** para cada uma. O JeffWrite oferece-se depois para guardar a lista como um ficheiro .csv.
- **Utilizar uma Lista Existente**: escolha um ficheiro que já seja uma lista. Pode ser um ficheiro .csv ou um ficheiro de texto com tabulações (uma linha para cada pessoa, e a primeira linha contém os nomes das colunas), uma folha de cálculo (.xlsx, .xls ou .ods, do JeffCrunch ou de outro programa), ou um ficheiro de contactos (.vcf).
- **Selecionar a partir dos Contactos**: escolha um ficheiro de contactos (.vcf). A maioria das agendas e telemóveis consegue guardar os contactos num ficheiro desse tipo.

Depois clique em **Correspondência > Editar Lista de Destinatários** para verificar a lista:

- Desmarque uma linha para deixar essa pessoa de fora.
- Clique num cabeçalho de coluna para ordenar.
- Escreva em **Filtrar destinatários** para mostrar apenas as linhas que contêm as suas palavras.
- **Distinguir os destinatários por** escolhe a coluna que distingue uma pessoa de outra, como um endereço de correio eletrónico. Quando volta a abrir o documento e a lista é lida, as pessoas que desmarcou continuam de fora, mesmo que a lista tenha outra ordem ou pessoas novas. Com **Colocar na lista**, as pessoas são lembradas pelo número da sua linha no ficheiro da lista, mesmo que tenha ordenado ou filtrado as linhas nesta janela. Se a lista tiver mudado de forma que isto já não funcione, o JeffWrite avisa e inclui todas as pessoas que não conseguiu identificar. O documento guarda uma forma codificada de cada endereço, e não o próprio endereço, mas quem tiver o documento e adivinhar um endereço ainda pode testar essa suposição. **Inspecionar Documento** (Ficheiro > Informações) remove o registo de quem ficou de fora.
- **Localizar Duplicados** desmarca as repetições.
- **Validar Endereços** colore as linhas que não têm morada ou que têm um código postal com aspeto incorreto.

## Coloque os dados na carta

Um **campo de impressão em série** é um marcador de posição. Mostra o nome de uma coluna entre sinais de aspas angulares, como «Nome». Em cada cópia, é substituído pelo dado dessa pessoa.

Clique onde o dado deve ficar e use **Correspondência > Escrever e Inserir Campos**:

- **Inserir Campo de Impressão em Série** lista as colunas da sua lista. Escolha uma.
  - **Código de Barras a partir de Campo**, na mesma lista, coloca um código de barras cujo valor é uma coluna, como um número de peça ou o ISBN de 13 dígitos de um livro. Escolha a coluna, depois o tipo, e veja na janela **Inserir Código de Barras** (o que escrever lá é apenas um exemplo). A janela oferece apenas o que um campo consegue conter: o tipo, o tamanho, a altura das barras, os dígitos e o nível do QR. Deixa de fora o preço de um livro, um suplemento, um carácter de controlo e o fundo claro, que um campo não consegue indicar; o ISBN de um livro é um EAN-13. No documento, o campo é desenhado como um código de barras, com um exemplo do tipo até fazer a impressão em série; para alterar o tipo ou o aspeto, faça duplo clique nele ou clique com o botão direito e escolha **Editar Código de Barras**. Ao fazer a impressão em série, a cópia de cada pessoa recebe um código de barras com o valor próprio dessa pessoa, desenhado como uma imagem na linha, para que seja impresso e guardado em PDF como qualquer imagem. O valor é apenas um dado: aspas ou uma barra invertida nele não alteram nada. Se um valor estiver vazio, não há código de barras. Se um código não conseguir conter um valor (uma letra num EAN-13, ou mais de 500 caracteres num Code 128), a cópia mostra uma breve mensagem entre parênteses retos no seu lugar, e as outras cópias não são afetadas. Uma impressão em série cria no máximo 2 000 códigos de barras, contando cada valor que não possa ser representado; se for indicado que há demasiados, imprima para menos pessoas. Num `.docx` guardado, o campo é um campo DISPLAYBARCODE com a coluna lá dentro, pelo que se mantém ao guardar e voltar a abrir o ficheiro. Um campo com um valor escrito diretamente, ou de um tipo que o JeffWrite não desenha (como o UPC-E), fica como está. As mensagens de correio eletrónico colocam o valor como texto.
- **Bloco de Endereço** coloca uma morada postal completa: o nome, a empresa, a rua e a localidade. Escolha as linhas que pretende. Uma linha sem conteúdo é omitida. Também pode indicar um país a omitir, como o seu. A **Pré-visualização** mostra a morada da primeira pessoa.
- **Linha de Saudação** coloca uma linha como "Caro Dr. Fenwick,". Você escolhe as palavras. A primeira opção, "Caro Dr. Fenwick,", usa o tratamento da pessoa. Se uma pessoa não tiver tratamento, usa o primeiro e o último nome, como em "Cara Nora Pell,". Se uma pessoa não tiver nome, a saudação diz outra coisa, como "Caro Senhor ou Senhora,", que pode alterar.
- **Corresponder Campos** indica ao bloco de endereço e à linha de saudação qual a coluna que contém cada parte, quando os nomes das suas colunas são pouco habituais.

## Regras

Uma **regra** torna uma cópia diferente para algumas pessoas. Clique em **Regras** e escolha uma:

- **Se…Então…Senão**: mostra um texto quando uma coluna tem um valor e outro texto quando não tem. Por exemplo, se o Distrito for "Lisboa", escreva "Bem-vindo a casa".
- **Ignorar Registo Se**: deixa uma pessoa de fora quando uma coluna cumpre um teste definido, como quando o apelido está vazio.
- **Registo Seguinte** e **Registo Seguinte Se**: usam os dados da pessoa seguinte no mesmo lugar. As etiquetas usam isto para colocar uma pessoa diferente em cada etiqueta.
- **Número de Registo da Impressão em Série** coloca o número da linha da pessoa na lista. **Número de Sequência da Impressão em Série** conta as cópias feitas até ao momento.
- **Perguntar** e **Preencher**: perguntam-lhe algo quando concluir a impressão em série. Perguntar guarda a resposta com um nome. Preencher coloca a resposta no texto.
- **Definir Marcador**: atribui um valor a um nome. Uma referência cruzada a esse nome mostra o valor.

## Pré-visualize os resultados

Clique em **Correspondência > Pré-visualizar Resultados**. Abre-se um painel que mostra a página de uma pessoa. Use **Primeiro Registo**, **Registo Anterior**, **Registo Seguinte** e **Último Registo** para percorrer as pessoas, ou escreva um número. **Encontrar Destinatário** salta para a pessoa seguinte que tenha as suas palavras num dado.

**Realçar Campos de Impressão em Série** sombreia a cinzento os campos da página, para que veja onde estão. Os outros campos, como os números de página, não são sombreados. O sombreado aparece no ecrã e nunca é impresso.

O seu documento não é alterado. **Verificar Erros** avisa quando a lista não tem uma coluna para um campo, ou quando não fica incluída nenhuma pessoa.

## Concluir a impressão em série

Clique em **Correspondência > Concluir e Unir**:

- **Editar Documentos Individuais** cria um novo documento com a cópia de cada pessoa. Pode alterá-lo antes de imprimir.
- **Imprimir Documentos** cria esse documento e abre a página **Imprimir**.
- **Guardar como PDF** guarda todas as cópias num único ficheiro PDF.
- **Enviar Mensagens de Correio Eletrónico** cria um ficheiro de mensagem .eml para cada pessoa, numa pasta que escolher. Escolha a coluna que contém os endereços de correio eletrónico e escreva um assunto. O JeffWrite não envia as mensagens. Abra os ficheiros no seu programa de correio eletrónico.
- **Guardar Cada um como Ficheiro** cria um ficheiro para cada pessoa, numa pasta que escolher, em formato .docx ou PDF. Dê nome aos ficheiros com colunas entre chavetas, como {Apelido}, {Nome}; um nome usado duas vezes recebe um número. Uma caixa mostra os primeiros nomes e quantos ficheiros serão criados. É-lhe pedida confirmação antes de mais de 200 ficheiros, e uma vez se já existirem ficheiros com os mesmos nomes na pasta. Clique em **Cancelar** para parar entre ficheiros: os ficheiros já criados mantêm-se; fechar a janela tem o mesmo efeito. Um ficheiro que seja uma ligação para outro ficheiro é substituído por um novo, e o ficheiro para o qual apontava não é alterado. Os ficheiros contêm a carta de uma só pessoa: não contêm a lista, a sua localização nem quem ficou de fora. Uma impressão em série cria no máximo 100 000 ficheiros.

As pessoas que desmarcou na lista ficam de fora.

Um ficheiro de mensagem só é criado para uma pessoa cujo endereço seja um único endereço de correio eletrónico. Se um endereço tiver dois (com uma vírgula ou um ponto e vírgula entre eles), tiver um nome junto, ou não tiver domínio, essa pessoa não recebe mensagem, e o JeffWrite indica quantas ficaram sem mensagem. Um assunto longo é guardado em partes que os programas de correio eletrónico voltam a juntar.

A impressão em série mostra uma janela de progresso, e **Cancelar** a interrompe. Se uma regra fizer uma pergunta (Perguntar ou Preencher), é-lhe colocada cada pergunta diferente uma vez, até 20 perguntas. Depois disso, as restantes usam as respostas predefinidas. Se cancelar uma pergunta, a impressão em série inteira para. A impressão em série para, e informa, se fosse criar mais de 10 000 páginas, para que um documento muito longo com uma lista muito grande não demore horas. Nesse caso, faça a impressão em série por partes da lista.

## Envelopes

Clique em **Correspondência > Envelopes**. Escreva a morada de entrega e a sua morada de remetente, e escolha o tamanho do envelope na lista. Os tamanhos são indicados pela largura e altura. Clique em **Adicionar ao Documento** para colocar o envelope numa página própria no início do documento, ou em **Imprimir** para o imprimir. A página tem a largura do lado maior do envelope, que é a forma como a maioria das impressoras recebe um envelope.

Para criar envelopes para uma impressão em série, clique em **Iniciar Correspondência > Envelopes**. Cada envelope tem um bloco de endereço.

## Etiquetas

Clique em **Correspondência > Etiquetas**. Escreva a morada e escolha a folha na lista. Uma folha é designada pelo número de etiquetas que contém e pelo tamanho de cada etiqueta. Se a sua folha não estiver na lista, escolha **Folha personalizada** e escreva as medidas: quantas etiquetas cabem na horizontal e na vertical, o tamanho de uma etiqueta, o espaço acima e ao lado da primeira etiqueta, e a distância entre as etiquetas.

- **Página completa da mesma etiqueta** coloca a mesma morada em todas as etiquetas da folha.
- **Etiqueta única** coloca a morada numa só etiqueta, para aproveitar uma folha já parcialmente usada. Indique a linha e a coluna.

Clique em **Novo Documento** para criar um documento que possa ver e alterar, ou em **Imprimir** para o imprimir.

Para criar etiquetas para uma impressão em série, clique em **Iniciar Correspondência > Etiquetas**. A primeira etiqueta tem um bloco de endereço, e cada uma das outras começa com uma regra **Registo Seguinte**. Se alterar a primeira etiqueta, clique em **Atualizar Etiquetas** para copiar a alteração para todas as outras.

## O que o ficheiro guarda

O tipo de impressão, o nome e o local da lista, o assunto e a coluna do correio eletrónico são guardados no seu ficheiro .docx. As pessoas que desmarcou também ficam guardadas, como códigos curtos da coluna que escolheu, e não pelos seus nomes ou endereços (só um ficheiro .docx guarda isto; Verificar Compatibilidade indica-o para os outros tipos). As escolhas de Corresponder Campos também são guardadas. Uma lista é lida novamente do seu ficheiro sempre que é utilizada. Se o ficheiro não estiver onde estava, o JeffWrite procura ao lado do seu documento e, depois, pede-lhe que o escolha.

Uma lista pode ter até 1000 colunas e 4 000 000 de células no total, e uma impressão em série usa as primeiras 100 000 pessoas. Se um ficheiro tiver mais do que isso, o JeffWrite usa o que cabe e avisa. As linhas sem conteúdo são omitidas da lista.

## Consulte também

- [Utilizar campos](fields)
- [Imprimir](print)
- [Criar e alterar tabelas](tables)
