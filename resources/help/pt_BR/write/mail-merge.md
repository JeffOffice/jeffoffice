# Mala direta, envelopes e etiquetas
<!-- keywords: mala direta, mesclagem, correspondências, destinatários, lista de endereços, carta modelo, etiquetas, envelopes, campo de mesclagem, bloco de endereço, linha de saudação, regras, se então senão, ignorar registro, próximo registro, visualizar resultados, concluir e mesclar, diretório, mensagens de email, csv, xlsx, ods, vcard -->

Uma **mala direta** cria muitas cópias de uma mesma carta, cada uma com os dados de uma pessoa diferente. Você escreve a carta uma vez. Guarda os dados das pessoas em uma **lista**. O JeffWrite coloca o nome e o endereço da primeira pessoa na primeira cópia, os da segunda pessoa na segunda cópia, e assim por diante.

As ferramentas estão na guia **Correspondências**. Para um passo a passo guiado, clique em **Correspondências > Iniciar Mala Direta > Assistente de Mala Direta Passo a Passo**.

## Escolha o que você está criando

Clique em **Correspondências > Iniciar Mala Direta** e escolha uma opção:

- **Cartas**: uma cópia da página para cada pessoa. Cada cópia começa em uma nova página.
- **Mensagens de Email**: uma mensagem para cada pessoa. Veja "Enviar mensagens de email" mais abaixo.
- **Envelopes** e **Etiquetas**: uma página preparada para eles. Veja as seções abaixo.
- **Diretório**: uma única lista de todas as pessoas, sem quebra de página entre elas. Use para uma lista telefônica ou um catálogo.
- **Limpar Tipo de Mesclagem** torna o documento um documento comum novamente.

O JeffWrite guarda o tipo de mala direta e o nome da lista dentro do arquivo quando você salva. Ao abrir o arquivo de novo, a mala direta continua lá.

## Escolha os destinatários

Um **destinatário** é uma pessoa que recebe uma cópia. Clique em **Correspondências > Selecionar Destinatários**:

- **Digitar uma Nova Lista**: digite as pessoas na janela que abrir. Clique em **Nova Entrada** para cada uma. Depois, o JeffWrite oferece salvar a lista como um arquivo .csv.
- **Usar uma Lista Existente**: escolha um arquivo que já seja uma lista. Pode ser um arquivo .csv ou um arquivo de texto com tabulações (uma linha para cada pessoa, e a primeira linha com os nomes das colunas), uma planilha (.xlsx, .xls ou .ods, do JeffCrunch ou de outro programa) ou um arquivo de contatos (.vcf).
- **Selecionar dos Contatos**: escolha um arquivo de contatos (.vcf). A maioria das agendas de endereços e celulares pode salvar os contatos nesse formato.

Depois, clique em **Correspondências > Editar Lista de Destinatários** para conferir a lista:

- Desmarque uma linha para deixar essa pessoa de fora.
- Clique no cabeçalho de uma coluna para classificar.
- Digite em **Filtrar destinatários** para mostrar apenas as linhas que têm as suas palavras.
- **Localizar Duplicatas** desmarca as repetições.
- **Validar Endereços** colore as linhas que não têm endereço ou que têm um CEP com aparência incorreta.

## Coloque os dados na carta

Um **campo de mesclagem** é um espaço reservado. Ele mostra o nome de uma coluna entre sinais de aspas angulares duplas, como «Nome». Em cada cópia, ele é substituído pelo dado daquela pessoa.

Clique onde o dado deve aparecer e use **Correspondências > Gravar e Inserir Campos**:

- **Inserir Campo de Mesclagem** lista as colunas da sua lista. Escolha uma.
  - **Código de Barras a partir do Campo**, na mesma lista, insere um código de barras cujo valor vem de uma coluna, como um número de peça ou o ISBN de 13 dígitos de um livro. Escolha a coluna e depois o tipo, e veja a janela **Inserir Código de Barras** (o que você digita ali é apenas um exemplo). A janela oferece somente o que um campo pode conter: o tipo, o tamanho, a altura das barras, os dígitos e o nível do QR. Ela deixa de fora o preço de um livro, o complemento, o caractere de verificação e o fundo transparente, que um campo não consegue indicar; o ISBN de um livro é um EAN-13. No documento, o campo mostra o nome da coluna entre aspas angulares. Na mesclagem, a cópia de cada pessoa recebe um código de barras com o valor próprio dela, desenhado como uma imagem na linha, para que seja impresso e salvo em PDF como qualquer imagem. O valor é apenas um dado: aspas ou barras invertidas dentro dele não mudam nada. Se o valor estiver vazio, não há código de barras. Se um código não puder conter o valor (uma letra em um EAN-13, ou mais de 500 caracteres em Code 128), a cópia mostra uma mensagem curta entre colchetes no lugar dele, e as demais cópias não são afetadas. Uma mesclagem cria no máximo 2.000 códigos de barras, contando cada valor que não puder ser representado; mescle menos pessoas se ela indicar que há muitos. Em um `.docx` salvo, o campo é um campo DISPLAYBARCODE com a coluna dentro, então ele é mantido ao salvar e reabrir o arquivo. Um campo com um valor digitado ou de um tipo que o JeffWrite não desenha (UPC-E, por exemplo) fica como está. Mesclagens de email colocam o valor como texto.
- **Bloco de Endereço** insere um endereço postal completo: o nome, a empresa, a rua e a cidade. Escolha as linhas que quiser. Uma linha sem conteúdo é omitida. Você também pode indicar um país a omitir, como o seu próprio. **Visualizar** mostra o endereço da primeira pessoa.
- **Linha de Saudação** insere uma linha como "Prezado Dr. Fenwick,". Você escolhe as palavras. A primeira opção, "Prezado Dr. Fenwick,", usa o título da pessoa. Se a pessoa não tiver título, usa o primeiro e o último nome, como em "Prezado Nora Pell,". Se a pessoa não tiver nome, a saudação diz outra coisa, como "Prezado Senhor ou Senhora,", que você pode alterar.
- **Corresponder Campos** informa ao bloco de endereço e à linha de saudação qual coluna contém cada parte, quando os nomes das colunas são incomuns.

## Regras

Uma **regra** torna a cópia diferente para algumas pessoas. Clique em **Regras** e escolha uma:

- **Se…Então…Senão**: mostra um texto quando uma coluna tem valor e outro texto quando não tem. Por exemplo, se Estado for "SP", escreva "Bem-vindo de volta".
- **Ignorar Registro Se**: deixa uma pessoa de fora quando uma coluna tem um valor, como quando o sobrenome está vazio.
- **Próximo Registro** e **Próximo Registro Se**: usam os dados da pessoa seguinte no mesmo lugar. As etiquetas usam isso para colocar uma pessoa diferente em cada etiqueta.
- **Número do Registro de Mesclagem** insere o número da linha da pessoa na lista. **Número de Sequência de Mesclagem** conta as cópias feitas até ali.
- **Perguntar** e **Preenchimento**: fazem uma pergunta quando você conclui a mesclagem. Perguntar guarda a resposta com um nome. Preenchimento coloca a resposta no texto.
- **Definir Indicador**: dá a um nome um valor. Uma referência cruzada a esse nome mostra o valor.

## Visualize os resultados

Clique em **Correspondências > Visualizar Resultados**. Abre-se um painel que mostra a página de uma pessoa. Use **Primeiro Registro**, **Registro Anterior**, **Próximo Registro** e **Último Registro** para percorrer as pessoas, ou digite um número. **Localizar Destinatário** vai até a próxima pessoa que tiver as suas palavras em algum dado.

**Realçar Campos de Mesclagem** sombreia os campos de mesclagem na página em cinza, para que você veja onde estão. Outros campos, como números de página, não são sombreados. O sombreamento aparece na tela e nunca é impresso.

Seu documento não é alterado. **Verificar Erros** avisa quando a lista não tem uma coluna para um campo, ou quando nenhuma pessoa está incluída.

## Conclua a mesclagem

Clique em **Correspondências > Concluir e Mesclar**:

- **Editar Documentos Individuais** cria um novo documento com a cópia de cada pessoa. Você pode alterá-lo antes de imprimir.
- **Imprimir Documentos** cria esse documento e abre a página **Imprimir**.
- **Salvar como PDF** salva todas as cópias em um único arquivo PDF.
- **Enviar Mensagens de Email** cria um arquivo de mensagem .eml para cada pessoa, em uma pasta que você escolhe. Escolha a coluna que contém os endereços de email e digite um assunto. O JeffWrite não envia as mensagens. Abra os arquivos no seu programa de email.

As pessoas que você desmarcou na lista ficam de fora.

Um arquivo de mensagem é criado somente para a pessoa cujo endereço contém um único endereço de email. Se um endereço tiver dois (com vírgula ou ponto e vírgula entre eles), tiver um nome junto ou não tiver domínio, essa pessoa não recebe mensagem, e o JeffWrite informa quantas foram. Um assunto longo é salvo em partes que os programas de email reúnem novamente.

A mesclagem mostra uma janela de andamento, e **Cancelar** a interrompe. Se uma regra fizer uma pergunta (Perguntar ou Preenchimento), você será questionado uma vez por cada pergunta diferente, até 20 perguntas. Depois disso, as demais usam as respostas padrão. Se você cancelar uma pergunta, a mesclagem inteira para. A mesclagem para e informa isso se for gerar mais de 10.000 páginas, para que um documento muito longo com uma lista muito longa não leve horas. Mescle uma parte da lista de cada vez.

## Envelopes

Clique em **Correspondências > Envelopes**. Digite o endereço de entrega e o seu endereço de remetente, e escolha o tamanho do envelope na lista. Os tamanhos são indicados pela largura e pela altura. Clique em **Adicionar ao Documento** para colocar o envelope em uma página própria no início do documento, ou em **Imprimir** para imprimi-lo. A página tem a largura do lado maior do envelope, que é a forma como a maioria das impressoras recebe um envelope.

Para criar envelopes em uma mala direta, clique em **Iniciar Mala Direta > Envelopes**. Cada envelope terá um bloco de endereço.

## Etiquetas

Clique em **Correspondências > Etiquetas**. Digite o endereço e escolha a folha na lista. Uma folha é indicada pela quantidade de etiquetas que tem e pelo tamanho de cada uma. Se a sua folha não estiver na lista, escolha **Sua própria folha** e digite as medidas: quantas etiquetas cabem na horizontal e na vertical, o tamanho de uma etiqueta, o espaço acima e ao lado da primeira etiqueta e a distância entre as etiquetas. Use centímetros ou milímetros, como indicado no JeffWrite.

- **Página inteira com a mesma etiqueta** coloca o mesmo endereço em todas as etiquetas da folha.
- **Etiqueta única** coloca o endereço em uma só etiqueta, para que você aproveite uma folha já parcialmente usada. Informe a linha e a coluna.

Clique em **Novo Documento** para criar um documento que você pode ver e alterar, ou em **Imprimir** para imprimi-lo.

Para criar etiquetas em uma mala direta, clique em **Iniciar Mala Direta > Etiquetas**. A primeira etiqueta terá um bloco de endereço, e cada uma das outras começará com uma regra de **Próximo Registro**. Se você alterar a primeira etiqueta, clique em **Atualizar Etiquetas** para copiar a alteração para todas as outras.

## O que o arquivo guarda

O tipo de mala direta, o nome e o local da lista, o assunto e a coluna de email são salvos no seu arquivo .docx. As pessoas que você desmarcou e as escolhas de Corresponder Campos não são salvas. Elas são solicitadas novamente depois que você abre o arquivo. Uma lista é lida novamente do seu arquivo toda vez. Se o arquivo não estiver mais no local original, o JeffWrite procura ao lado do seu documento e, depois, pede que você o escolha.

Uma lista pode ter até 1.000 colunas e 4.000.000 de células no total, e uma mesclagem usa as primeiras 100.000 pessoas. Se um arquivo tiver mais que isso, o JeffWrite usa o que cabe e informa. Linhas sem nenhum conteúdo são omitidas da lista.

## Veja também

- [Usar campos](fields)
- [Imprimir](print)
- [Criar e alterar tabelas](tables)
